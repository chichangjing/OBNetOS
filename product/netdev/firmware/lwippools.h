
#if MEM_USE_POOLS
LWIP_MALLOC_MEMPOOL_START
LWIP_MALLOC_MEMPOOL(16, 256) 
LWIP_MALLOC_MEMPOOL(12, 512) 
LWIP_MALLOC_MEMPOOL(4, 1536) 
LWIP_MALLOC_MEMPOOL_END
#endif /* MEM_USE_POOLS */
 
/* Optional: Your custom pools can go here if you would like to use
 * lwIP's memory pools for anything else.
 */
//LWIP_MEMPOOL(SYS_MBOX, 22, 100, "SYS_MBOX")

