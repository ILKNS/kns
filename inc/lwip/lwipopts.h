#define	LWIP_STATS	0
#define	LWIP_TCP	1
#define	NO_SYS		1
#define LWIP_RAW	0
#define LWIP_UDP	0
#define IP_REASSEMBLY	0
#define IP_FRAG		0
#define LWIP_NETCONN	0

#define MEM_LIBC_MALLOC 1
#define MEMP_MEM_MALLOC 1

#undef LWIP_DEBUG
#define	TCP_CWND_DEBUG		LWIP_DBG_OFF
#define	TCP_DEBUG		LWIP_DBG_OFF
#define	TCP_FR_DEBUG		LWIP_DBG_OFF
#define	TCP_INPUT_DEBUG		LWIP_DBG_OFF
#define	TCP_OUTPUT_DEBUG	LWIP_DBG_OFF
#define	TCP_QLEN_DEBUG		LWIP_DBG_OFF
#define	TCP_RST_DEBUG		LWIP_DBG_OFF
#define	TCP_RTO_DEBUG		LWIP_DBG_OFF
#define	TCP_WND_DEBUG		LWIP_DBG_OFF

#define LWIP_IX

#include <linux/byteorder/generic.h>

#define LWIP_PLATFORM_BYTESWAP	1
#define LWIP_PLATFORM_HTONS(x) ___htons(x)
#define LWIP_PLATFORM_NTOHS(x) ___ntohs(x)
#define LWIP_PLATFORM_HTONL(x) ___htonl(x)
#define LWIP_PLATFORM_NTOHL(x) ___ntohl(x)

#define LWIP_WND_SCALE 1
#define TCP_RCV_SCALE 7
#define TCP_SND_BUF 65536
#define TCP_MSS 1460
#define TCP_WND (2048 * TCP_MSS)

#define CHECKSUM_CHECK_IP               0
#define CHECKSUM_CHECK_TCP              0
#define TCP_ACK_DELAY (1 * ONE_MS)
#define RTO_UNITS (500 * ONE_MS)

/* EdB 2014-11-07 */
#define LWIP_NOASSERT
#define LWIP_EVENT_API 1
#define LWIP_NETIF_HWADDRHINT 1
