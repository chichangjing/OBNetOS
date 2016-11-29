/*
 *  rc_ssl.c (SSL adapter for OpenSSL stack)
 *
 *  This is a part of the RapidControl SDK source code library.
 *
 *  Copyright (C) 1999 Rapid Logic, Inc.
 *  All rights reserved.
 *
 */

/*

$History: rc_ssl.c $
 * 
 * *****************  Version 6  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments


 */

#include "rc_options.h"

#ifdef __RLI_SSL_ENABLED__


#include <stdio.h>

#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_msghdlr.h"
#include "rc_convert.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_database.h"
#include "rc_occustom.h"

/*elp begin put win32 protos to resolve warnings*/
#include "rc_socks.h"
/*elp end*/

/* ssl includes: */

#include "rc_ssl_defs.h"


/*
typedef struct ssl_socket_handle {
    REAL_OS_SPECIFIC_SOCKET_HANDLE sock;
	SSL_CTX *sslctx;
    SSL *ssl;
} ssl_socket_handle;

*/

static char *rc_server_cert = DEFAULT_SSL_SERVER_CERT_PATH; 
static char *rc_CAfile = DEFAULT_SSL_CA_FILE;
static char *rc_CApath = DEFAULT_SSL_CA_PATH;
static char *rc_cipher_list = DEFAULT_SSL_CIPHER_LIST;

static int ssl_cache_sessions; /* true if caching sessions */
 
/* local functions in this file: */
BIO_METHOD *BIO_f_rc() ;
ssl_socket_handle * rc_ssl_table2hand(int table) ;
int MS_CALLBACK rc_ssl_verify_callback(int ok, X509_STORE_CTX *ctx);
ssl_socket_handle *rc_new_ssl_handle();
RLSTATUS rc_do_ssl_setup(ssl_socket_handle *hand);

int rc_ssl_bio_write(BIO *b, const char *in, int inl) ;
int rc_ssl_bio_read(BIO *b, void *out, int outl) ;
int rc_ssl_bio_free(BIO *b);
int rc_ssl_bio_new(BIO *b);
long rc_ssl_bio_ctrl(BIO *b, int cmd, long num, char *ptr);
void rc_ssl_clear_SSL_table(int sock);
#ifdef DO_RSA
static RSA MS_CALLBACK *rsa_cb(SSL *s, int is_export, int keylength);
#endif
#ifdef DO_DH
int  rc_ssl_set_DH(ssl_socket_handle *hand)
#endif 

static BIO_METHOD OS_SPECIFIC_SSL_METHOD = {
	BIO_TYPE_SOCKET,
	"RapidControl",
	rc_ssl_bio_write,  /* RLI needs to make our own (new) FILEMGR and ANSIFS calls here */
	rc_ssl_bio_read,
	NULL, /* puts method .. XXX do we need to support these?*/
	NULL, /* gets method */
	rc_ssl_bio_ctrl, /* ctrl method */
	rc_ssl_bio_new,
	rc_ssl_bio_free
};



static ssl_socket_handle *rc_SSL_table[SSL_TABLE_MAX];


/* find the real socket. */
/* returns 0 on error, positive int otherwise */
OS_SPECIFIC_SOCKET_HANDLE rc_ssl_table2sock(int table) {
	ssl_socket_handle *hand = rc_ssl_table2hand(table);
	if (!hand) return 0;
	return hand->sock;
}


RLSTATUS rc_do_ssl_socket(OS_SPECIFIC_SOCKET_HANDLE *sock)
{
	RLSTATUS rc = OK;
	ssl_socket_handle *hand;

	SSL_library_init();
	/* SSL_load_error_strings(); */

#ifdef SSL_DO_SESSION_CACHING
	ssl_cache_sessions = 1;
#endif

	if (!(hand = rc_new_ssl_handle(*sock)))
		return ERROR_MEMMGR_NO_MEMORY;



	return rc;

}


RLSTATUS rc_ssl_close(OS_SPECIFIC_SOCKET_HANDLE handle)
{
	RLSTATUS rc;
	ssl_socket_handle *hand;

#ifdef DEBUG
	printf("rc_ssl_close(%d)\n",handle);
#endif

	if (!(hand = rc_ssl_table2hand(handle)))
		return(REAL_OS_SPECIFIC_SOCKET_CLOSE(handle));

	BIO_flush(hand->s_to_c);

	/* clean up SSL junk: */
	BIO_free_all(hand->c_to_s);
	hand->c_to_s = NULL;
	BIO_free_all(hand->c_to_s);
	hand->s_to_c = NULL;
	BIO_free_all(hand->s_bio);
	hand->s_bio = NULL;

	/* close real socket: */
	rc = REAL_OS_SPECIFIC_SOCKET_CLOSE(hand->sock);


	if (!ssl_cache_sessions) {
		/* really close it */

		SSL_free(hand->ssl);

		SSL_CTX_free(hand->sslctx);

		/* clear the table entry */
		rc_ssl_clear_SSL_table(hand->sock);

		/* and free table entry mem */
		memset(hand,0,sizeof(ssl_socket_handle));
		OS_SPECIFIC_FREE(hand);
	}
	else {
		/* saving cached session...
		   clear out everything except the ctx and ssl pointers */
		hand->sock = -1;

		/* diddle ssl struct too: */
		if(hand->ssl)/*bad pointer error handling -elp*/
		{
		hand->ssl->rbio = NULL;
		hand->ssl->wbio = NULL;
		hand->ssl->bbio = NULL;
		}
#ifdef DEBUG /*elp*/
		else
			printf("bad handle, hand->ssl\n");
#endif
		

	}

	return rc;
}

RLSTATUS rc_ssl_accept(OS_SPECIFIC_SOCKET_HANDLE *soc,
	OS_SPECIFIC_SOCKET_HANDLE *accSoc, ubyte4 port)
{
	RLSTATUS status = OK;
	ssl_socket_handle *hand;
	OS_SPECIFIC_SOCKET_HANDLE realsock;

	/* do real accept first: */
	do {
		hand = rc_ssl_table2hand(*soc);
		if (!hand) return(REAL_OS_SPECIFIC_SOCKET_ACCEPT(soc,accSoc,port));

		realsock = hand->sock;
		status = REAL_OS_SPECIFIC_SOCKET_ACCEPT(&realsock,accSoc,port);
		if (status < OK) break;
#ifdef DEBUG
		printf("rc_ssl_accept(%d) acc sock %d\n",*soc,*accSoc);
#endif

		/* if we don't already have an SSL ctx for this connection, make one: */
		if (!rc_ssl_table2hand(*accSoc)) {

			/* now make a new SSL handle to be the new "socket" */
			hand = rc_new_ssl_handle(*accSoc);

			/* set up SSL on it: */
			status = rc_do_ssl_setup(hand);
		}
	
		if (status < OK) {
			char *pMsg;

			MsgHdlr_RetrieveOpenControlMessage(SYS_ERROR_SOCKET_ACCEPT, &pMsg);
	     	OS_SPECIFIC_LOG_ERROR(kSevereError, pMsg);
		}
		else
			status = OK;
	} while(0);

	return status;
}



ubyte4 rc_ssl_GetClientAddr(environment *p_envVar)
{
	ubyte4 ret;
	OS_SPECIFIC_SOCKET_HANDLE h;
	ssl_socket_handle *hand = rc_ssl_table2hand(p_envVar->sock);
	if (!hand) return REAL_OS_SPECIFIC_GET_ADDR(p_envVar);

	/* save old (fake) sock and put real one in place: */
	h = p_envVar->sock;
	p_envVar->sock = rc_ssl_table2sock(p_envVar->sock);
	if (p_envVar->sock < (OS_SPECIFIC_SOCKET_HANDLE)0)
		ret = 0;
	else
		ret = REAL_OS_SPECIFIC_GET_ADDR(p_envVar);

	/* switch back: */
	p_envVar->sock = h;

	return ret;
}

ubyte4 rc_ssl_GetClientAddrFromSock(OS_SPECIFIC_SOCKET_HANDLE sock)
{
	ubyte4 ret;
	ssl_socket_handle *hand = rc_ssl_table2hand(sock);
	if (!hand) return SOCKET_GetClientsAddr(sock);

	ret = SOCKET_GetClientsAddr(rc_ssl_table2sock(sock));
	return ret;
}

RLSTATUS rc_ssl_SocketRead(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, 
	Counter BufSize)
{
	ssl_socket_handle *hand;
	RLSTATUS i;

	if (!(hand = rc_ssl_table2hand(sock)))
		return(REAL_OS_SPECIFIC_SOCKET_READ(sock,pBuf,BufSize));

	i = BIO_read(hand->s_bio,pBuf,BufSize);
	if (i < 0) {
		if (BIO_should_retry(hand->s_bio)) {
			i = 0; /* return 0 to caller so he reads again */
		}
	}
	return i;
}

RLSTATUS rc_ssl_SocketWrite(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf, 
	Counter BufLen)
{
	ssl_socket_handle *hand;
	RLSTATUS i;

	if (!(hand = rc_ssl_table2hand(sock)))
		return(REAL_OS_SPECIFIC_SOCKET_WRITE(sock,pBuf,BufLen));

	i = BIO_write(hand->s_bio,pBuf,BufLen);
	if (i < 0) {
		if (BIO_should_retry(hand->s_bio)) {
			i = 0; /* return 0 to caller so he reads again */
		}
	}
	if (i >= OK)
         return OK;
	return i;
}

RLSTATUS rc_ssl_SocketDataAvailable(OS_SPECIFIC_SOCKET_HANDLE sock,
	 sbyte4 timeout)
{
	RLSTATUS rc;
	ssl_socket_handle *hand;

	if (!(hand = rc_ssl_table2hand(sock)))
		return(REAL_OS_SPECIFIC_SOCKET_DATA_AVAILABLE(sock,timeout));

	/* check openssl stack first:*/
	rc = BIO_pending(hand->s_bio);
	if (rc <= 0) { 
		/* nothing in BIO, check TCP stack */
		rc = REAL_OS_SPECIFIC_SOCKET_DATA_AVAILABLE(rc_ssl_table2sock(sock),
			timeout);
	}
	return rc;
}



/*******************************************************************
*
* local functions
*
********************************************************************/

BIO_METHOD *BIO_f_rc() {
	return(&OS_SPECIFIC_SSL_METHOD);
}




/* XXX this assumes that 0 will never be a valid real socket! */
ssl_socket_handle * rc_ssl_table2hand(int table) {
	int i;

	for (i = 0; i < SSL_TABLE_MAX; i++)
	{
		if (rc_SSL_table[i] && (int)rc_SSL_table[i]->sock == table)/* elp fixed signed/unsigned mismatch warning*/
			return rc_SSL_table[i];
	}
	return(NULL);
}

void rc_ssl_clear_SSL_table(int sock) {
	int i;
	
	for (i = 0; i < SSL_TABLE_MAX; i++) {
		if (rc_SSL_table[i] && (int)rc_SSL_table[i]->sock == sock)/* elp fixed signed/unsigned mismatch*/
			rc_SSL_table[i] = NULL; /* zero it */
	}
}


int MS_CALLBACK rc_ssl_verify_callback(int ok, X509_STORE_CTX *ctx)
{
	char *s, buf[256];

	s=X509_NAME_oneline(X509_get_subject_name(ctx->current_cert),buf,256);
	if (!s && ok == 0) {
		switch (ctx->error) {
	        case X509_V_ERR_CERT_NOT_YET_VALID:
			case X509_V_ERR_CERT_HAS_EXPIRED:
			case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
				ok=1;
		}
	}
	return ok;
}

	
ssl_socket_handle *rc_new_ssl_handle(OS_SPECIFIC_SOCKET_HANDLE sock)
{
	ssl_socket_handle *ret;
	int i, numfree, mostdisused, mostdisusedslot;
	ubyte4 addr;

	/* get the client addr.  Search the existing connections-
	 if there is one that's marked for reuse (sock is -1 but
	 valid ssl and sslctx structs) that has the same client
	 addr, then set it's socket to sock and return it. */

	do {
		ret = NULL;
		numfree = 0;
		mostdisused = 0;
		mostdisusedslot = -1;

		addr = SOCKET_GetClientsAddr(sock);
		if (addr > 0) { 
			/* a real connection, not a fake one.*/

			/* look for cached session:*/
			for(i = 0; i < SSL_TABLE_MAX; i++) {
				if (rc_SSL_table[i] && rc_SSL_table[i]->sock == -1) {
					if (rc_SSL_table[i]->ssl && rc_SSL_table[i]->sslctx) {
						if (rc_SSL_table[i]->client_addr == addr) {
							/* got a cached session! */
							ret = rc_SSL_table[i];
#ifdef DEBUG
							printf("SSL using cached session #%d\n",i);
#endif
						}
						else {
							/* add one to disused count:*/
							rc_SSL_table[i]->disusecount++;
							if (rc_SSL_table[i]->disusecount > mostdisused) {
								mostdisused = rc_SSL_table[i]->disusecount;
								mostdisusedslot = i;
							}
						}
					}
					else {
						numfree++;
					}
				}
			}
			if (ret) break; /* found a cached session.*/
			
			if (mostdisused > 0 && numfree == 0) {
				/* no free slots, we will have to make one
				   set ssl_cache_sessions to close it fully, no matter what */
				int tmp = ssl_cache_sessions;
				ssl_cache_sessions = 0;
				rc_ssl_close((OS_SPECIFIC_SOCKET_HANDLE)rc_SSL_table[mostdisusedslot]);
	
				/* set it back to what it was */
				ssl_cache_sessions = tmp;
#ifdef DEBUG
				printf("SSL freeing old cached session #%d, disused %d\n",
					mostdisusedslot,mostdisused);
#endif
			}
		}

		/* allocate a new SSL struct off the table: */
		for(i = 0; i < SSL_TABLE_MAX; i++) {
			if (!rc_SSL_table[i]) {
				rc_SSL_table[i] = 
					(ssl_socket_handle*)
						OS_SPECIFIC_MALLOC(sizeof(ssl_socket_handle));
				if (!rc_SSL_table[i])
					return NULL;
				memset(rc_SSL_table[i],0,sizeof(ssl_socket_handle));
				break;
			}
		}
		if (i >= SSL_TABLE_MAX) {
			/* ran out of slots... try to deal by closing an earlier one */
			/* and using that.*/
			for(i = 0; i < SSL_TABLE_MAX; i++) {
				if (rc_SSL_table[i]->ssl) {
					/* not a fake slot */
					rc_ssl_close(rc_SSL_table[i]->sock);

					/* and free table entry mem */
#ifndef SSL_DO_SESSION_CACHING
					memset(hand,0,sizeof(ssl_socket_handle));
					OS_SPECIFIC_FREE(hand);
					/* and alloc a new one */
					rc_SSL_table[i] = 
						(ssl_socket_handle*)
							OS_SPECIFIC_MALLOC(sizeof(ssl_socket_handle));
					if (!rc_SSL_table[i])
						return NULL;
					memset(rc_SSL_table[i],0,sizeof(ssl_socket_handle));
#endif /* ! SSL_DO_SESSION_CACHING */
					ret = rc_SSL_table[i];
					break;
				}
			}
		}
		else {
			/* valid slot */
			ret = rc_SSL_table[i];
		}
#ifdef DEBUG
		printf("SSL allocating new session #%d\n",i);
#endif

	} while(0);
	if (ret) {
		ret->sock = sock; /* bind the real socket to it */
		ret->client_addr = addr; /* same with addr */
	}

	return ret;
}
		
RLSTATUS rc_do_ssl_setup(ssl_socket_handle *hand)
{
	RLSTATUS rc = OK;

	/* now do the SSL setup: */
	do {
		if (!hand->ssl || !hand->sslctx) {
			/* an initial ssl connection, not a cached session*/
			SSL_METHOD *meth;

			/* XXX allow SSLv2?  it has security holes.*/
			/* but Netscape won't talk plain SSLv3...*/
#ifdef SSL_USE_TLS
			meth = TLSv1_server_method();
#else
			meth = SSLv23_server_method();
#endif
			hand->sslctx = SSL_CTX_new(meth);
			if (!hand->sslctx) { rc = ERROR_SSL_CTX; break; }

			SSL_CTX_set_cipher_list(hand->sslctx,rc_cipher_list);

#ifdef DO_DH
			rc_ssl_set_DH(hand);
#endif /* DO_DH */

#ifdef DO_RSA
			/* RSA temp stuff: */
			SSL_CTX_set_tmp_rsa_callback(hand->sslctx,rsa_cb);
#endif 

			/* set server cert path:*/
			rc = SSL_CTX_use_certificate_file(hand->sslctx,rc_server_cert,
				SSL_FILETYPE_PEM);
			if (!rc) {rc = ERROR_SSL_SERVER_CERT; break; }

			rc = SSL_CTX_use_PrivateKey_file(hand->sslctx,rc_server_cert,
				SSL_FILETYPE_PEM);
			if (!rc) {rc = ERROR_SSL_SERVER_CERT; break; }


			hand->ssl = SSL_new(hand->sslctx);

			/* zero session:*/
			SSL_set_session(hand->ssl,NULL);
		} /* end of new session stuff */

		/* set up the BIO stuff:*/
		hand->c_to_s = BIO_new(BIO_f_rc());
		hand->s_to_c = BIO_new(BIO_f_rc());
		hand->s_bio = BIO_new(BIO_f_ssl());

		SSL_set_accept_state(hand->ssl);

		SSL_set_bio(hand->ssl,hand->c_to_s,hand->s_to_c);
		BIO_set_ssl(hand->s_bio,hand->ssl,BIO_NOCLOSE);

		/* set the fd:*/
		BIO_set_fd(hand->c_to_s,hand->sock,BIO_NOCLOSE);
		BIO_set_fd(hand->s_to_c,hand->sock,BIO_NOCLOSE);

	} while(0);
	return rc;
}

int rc_ssl_bio_new(BIO *b)
{
#ifdef DEBUG
	printf("in rc_ssl_bio_new\n");
#endif
	b->init=1; /* was 0 */
	b->num=0;
	b->ptr=NULL;
	b->flags=0;
	return(1);
}

int rc_ssl_bio_free(BIO *b)
{
#ifdef DEBUG
	printf("in rc_ssl_bio_free\n");
#endif
	if (b == NULL) return(0);
	if (b->shutdown) {
		if (b->init) {
			REAL_OS_SPECIFIC_SOCKET_CLOSE(b->num);
		}
		b->init=0;
		b->flags=0;
	}
	return(1);
}

long rc_ssl_bio_ctrl(BIO *b, int cmd, long num, char *ptr)
{
	long ret = 1;
	int *ip;

	switch (cmd) {
	case BIO_CTRL_RESET:
		num  =  0;
	case BIO_C_FILE_SEEK:
		ret = 0;
		break;
	case BIO_C_FILE_TELL:
	case BIO_CTRL_INFO:
		ret = 0;
		break;
	case BIO_C_SET_FD:
		b->num =  *((int *)ptr);
		b->shutdown = (int)num;
		b->init = 1;
		break;
	case BIO_C_GET_FD:
		if (b->init) {
			ip = (int *)ptr;
			if (ip !=  NULL)
				*ip = b->num;
			ret = b->num;
		}
		else
			ret =  -1;
		break;
	case BIO_CTRL_GET_CLOSE:
		ret = b->shutdown;
		break;
	case BIO_CTRL_SET_CLOSE:
		b->shutdown = (int)num;
		break;
	case BIO_CTRL_PENDING:
	case BIO_CTRL_WPENDING:
		ret = 0;
		break;
	case BIO_CTRL_DUP:
	case BIO_CTRL_FLUSH:
		ret = 1;
		break;
	default:
		ret = 0;
		break;
	}
	return(ret);
}

int rc_ssl_bio_read(BIO *b, void *out, int outl) {
	int rc;


#ifdef DEBUG
	/*printf("rc_ssl_bio_read(%d,x,%d) returns ",b->num,outl);
	  fflush(stdout); */
#endif
	rc = REAL_OS_SPECIFIC_SOCKET_READ((OS_SPECIFIC_SOCKET_HANDLE)b->num,
			(sbyte*)out,(Counter)outl);
#ifdef DEBUG
	/*printf("%d\n",rc); */
#endif

	BIO_clear_retry_flags(b);
	if (rc <= 0) {
		if (BIO_sock_should_retry(rc))
			BIO_set_retry_read(b);
	}
	return rc;
}

int rc_ssl_bio_write(BIO *b, const char *in, int inl) {
	int rc;

#ifdef DEBUG
	/*printf("rc_ssl_bio_write(%d,x,%d) returns ",b->num,inl);
	  fflush(stdout); */
#endif
	rc = REAL_OS_SPECIFIC_SOCKET_WRITE((OS_SPECIFIC_SOCKET_HANDLE)b->num,
			(sbyte*)in,(Counter)inl);
	/* to make ssleay happy:*/
	if(rc == OK)
		rc = inl;
#ifdef DEBUG
	/*printf("%d\n",rc); */
#endif

	return rc;
}


#ifdef DO_RSA
static RSA MS_CALLBACK *rsa_cb(SSL *s, int is_export, int keylength)
	{
	static RSA *rsa_tmp=NULL;

	if (rsa_tmp == NULL) {
		/*BIO_printf(bio_err,"Generating temp (%d bit) RSA key...",keylength);
		  (void)BIO_flush(bio_err); */
		rsa_tmp = RSA_generate_key(keylength,RSA_F4,NULL,NULL);
		/*BIO_printf(bio_err,"\n");
		  (void)BIO_flush(bio_err); */
	}
	return(rsa_tmp);
}
#endif //DO_RSA


#ifdef DO_DH
int  rc_ssl_set_DH(ssl_socket_handle *hand)
{
	/* diffie-hellman params:*/
	DSA *dsa;
	DH *dh;

#ifdef DEBUG
	printf("Generating DH key..."); fflush(stdout);
#endif

	dsa = DSA_generate_parameters(1024, NULL, 0, NULL, NULL, 0, NULL);
	dh = DSA_dup_DH(dsa);	
	DSA_free(dsa);
	/* important: SSL_OP_SINGLE_DH_USE to avoid small subgroup attacks */
	SSL_CTX_set_options(hand->sslctx, SSL_OP_SINGLE_DH_USE);

	SSL_CTX_set_tmp_dh(hand->sslctx, dh);
#ifdef DEBUG
	printf("done\n");
#endif


	DH_free(dh);
}
#endif /* DO_DH */
#endif /* __RLI_SSL_ENABLED__ */
