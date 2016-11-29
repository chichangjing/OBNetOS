
#ifndef SSL_DEFS_H
#define SSL_DEFS_H

#ifdef __RLI_SSL_ENABLED__

/* ssl includes: */

#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <../e_os.h>

#include "rc_os_spec.h"



/* wrapper for socket handle & ssl foo */
typedef struct ssl_socket_handle {
	OS_SPECIFIC_SOCKET_HANDLE sock;
	SSL_CTX *sslctx;
	SSL *ssl;
	BIO *s_bio, *c_to_s, *s_to_c;
	ubyte4 client_addr;
	int disusecount;
} ssl_socket_handle;

/* table mapping "socket" handles to the appropriate ssl_socket_handle struct*/
extern ssl_socket_handle *rc_SSL_table[];

/* number of entries in the rc_SSL_table:*/
#define SSL_TABLE_MAX 40

/* do ephemeral RSA?
// #define DO_RSA 
// do Anonymous Diffie-Hellman?
// #define DO_DH
// do session caching? */
#define SSL_DO_SESSION_CACHING

/* SSL debugging messages: */
#define DEBUG



/* Exported functions: */

#ifdef __cplusplus
extern "C" {
#endif

ssl_socket_handle * rc_ssl_table2hand(int table) ;
OS_SPECIFIC_SOCKET_HANDLE rc_ssl_table2sock(int table) ;
RLSTATUS rc_do_ssl_socket(OS_SPECIFIC_SOCKET_HANDLE *s);
ubyte4 rc_ssl_GetClientAddr(environment *p_envVar);
RLSTATUS rc_ssl_accept(OS_SPECIFIC_SOCKET_HANDLE *soc,
    OS_SPECIFIC_SOCKET_HANDLE *accSoc, ubyte4 port);
RLSTATUS rc_ssl_SocketRead(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf,
    Counter BufSize);
RLSTATUS rc_ssl_SocketWrite(OS_SPECIFIC_SOCKET_HANDLE sock, sbyte *pBuf,
    Counter BufLen);
RLSTATUS rc_ssl_close(OS_SPECIFIC_SOCKET_HANDLE handle);
RLSTATUS rc_ssl_SocketDataAvailable(OS_SPECIFIC_SOCKET_HANDLE sock,
	 sbyte4 timeout);

#ifdef __cplusplus
}
#endif

/* #define DEFAULT_SSL_CIPHER_LIST "RC4-SHA:DES-CBC3-SHA:DES-CBC-SHA"; */
#define DEFAULT_SSL_CIPHER_LIST "DEFAULT";

#define DEFAULT_SSL_SERVER_CERT_PATH "c:\\progra~1\\rapidc~1\\server.pem"
#define DEFAULT_SSL_CA_FILE ""
#define DEFAULT_SSL_CA_PATH ""



#endif /* __RLI_SSL_ENABLED__ */
#endif /* SSL_DEFS_H */
