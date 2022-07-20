#include "platform_def.h"
#include "luat_base.h"
#include "luat_malloc.h"
#include "luat_mcu.h"
#include "luat_rtos.h"
#include "dns_def.h"
#include "luat_network_adapter.h"
#include "net_lwip.h"



/* Socket protocol types (TCP/UDP/RAW) */
#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3

/*
 * Option flags per-socket. These must match the SOF_ flags in ip.h (checked in init.c)
 */
#define SO_REUSEADDR   0x0004 /* Allow local address reuse */
#define SO_KEEPALIVE   0x0008 /* keep connections alive */
#define SO_BROADCAST   0x0020 /* permit to send and to receive broadcast messages (see IP_SOF_BROADCAST option) */


/*
 * Additional options, not kept in so_options.
 */
#define SO_DEBUG        0x0001 /* Unimplemented: turn on debugging info recording */
#define SO_ACCEPTCONN   0x0002 /* socket has had listen() */
#define SO_DONTROUTE    0x0010 /* Unimplemented: just use interface addresses */
#define SO_USELOOPBACK  0x0040 /* Unimplemented: bypass hardware when possible */
#define SO_LINGER       0x0080 /* linger on close if data present */
#define SO_DONTLINGER   ((int)(~SO_LINGER))
#define SO_OOBINLINE    0x0100 /* Unimplemented: leave received OOB data in line */
#define SO_REUSEPORT    0x0200 /* Unimplemented: allow local address & port reuse */
#define SO_SNDBUF       0x1001 /* Unimplemented: send buffer size */
#define SO_RCVBUF       0x1002 /* receive buffer size */
#define SO_SNDLOWAT     0x1003 /* Unimplemented: send low-water mark */
#define SO_RCVLOWAT     0x1004 /* Unimplemented: receive low-water mark */
#define SO_SNDTIMEO     0x1005 /* send timeout */
#define SO_RCVTIMEO     0x1006 /* receive timeout */
#define SO_ERROR        0x1007 /* get error status and clear */
#define SO_TYPE         0x1008 /* get socket type */
#define SO_CONTIMEO     0x1009 /* Unimplemented: connect timeout */
#define SO_NO_CHECK     0x100a /* don't create UDP checksum */
#define SO_BINDTODEVICE 0x100b /* bind to device */

/*
 * Structure used for manipulating linger option.
 */
/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define  SOL_SOCKET  0xfff    /* options for socket level */


#define AF_UNSPEC       0
#define AF_INET         2
#if LWIP_IPV6
#define AF_INET6        10
#else /* LWIP_IPV6 */
#define AF_INET6        AF_UNSPEC
#endif /* LWIP_IPV6 */
#define PF_INET         AF_INET
#define PF_INET6        AF_INET6
#define PF_UNSPEC       AF_UNSPEC

#define IPPROTO_IP      0
#define IPPROTO_ICMP    1
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17
#if LWIP_IPV6
#define IPPROTO_IPV6    41
#define IPPROTO_ICMPV6  58
#endif /* LWIP_IPV6 */
#define IPPROTO_UDPLITE 136
#define IPPROTO_RAW     255

/* Flags we can use with send and recv. */
#define MSG_PEEK       0x01    /* Peeks at an incoming message */
#define MSG_WAITALL    0x02    /* Unimplemented: Requests that the function block until the full amount of data requested can be returned */
#define MSG_OOB        0x04    /* Unimplemented: Requests out-of-band data. The significance and semantics of out-of-band data are protocol-specific */
#define MSG_DONTWAIT   0x08    /* Nonblocking i/o for this operation only */
#define MSG_MORE       0x10    /* Sender will send more */
#define MSG_NOSIGNAL   0x20    /* Uninmplemented: Requests not to send the SIGPIPE signal if an attempt to send is made on a stream-oriented socket that is no longer connected. */


/*
 * Options for level IPPROTO_IP
 */
#define IP_TOS             1
#define IP_TTL             2
#define IP_PKTINFO         8

#if LWIP_TCP
/*
 * Options for level IPPROTO_TCP
 */
#define TCP_NODELAY    0x01    /* don't delay send to coalesce packets */
#define TCP_KEEPALIVE  0x02    /* send KEEPALIVE probes when idle for pcb->keep_idle milliseconds */
#define TCP_KEEPIDLE   0x03    /* set pcb->keep_idle  - Same as TCP_KEEPALIVE, but use seconds for get/setsockopt */
#define TCP_KEEPINTVL  0x04    /* set pcb->keep_intvl - Use seconds for get/setsockopt */
#define TCP_KEEPCNT    0x05    /* set pcb->keep_cnt   - Use number of probes sent for get/setsockopt */
#endif /* LWIP_TCP */

#if LWIP_IPV6
/*
 * Options for level IPPROTO_IPV6
 */
#define IPV6_CHECKSUM       7  /* RFC3542: calculate and insert the ICMPv6 checksum for raw sockets. */
#define IPV6_V6ONLY         27 /* RFC3493: boolean control to restrict AF_INET6 sockets to IPv6 communications only. */
#endif /* LWIP_IPV6 */

#if LWIP_UDP && LWIP_UDPLITE
/*
 * Options for level IPPROTO_UDPLITE
 */
#define UDPLITE_SEND_CSCOV 0x01 /* sender checksum coverage */
#define UDPLITE_RECV_CSCOV 0x02 /* minimal receiver checksum coverage */
#endif /* LWIP_UDP && LWIP_UDPLITE*/


#if LWIP_MULTICAST_TX_OPTIONS
/*
 * Options and types for UDP multicast traffic handling
 */
#define IP_MULTICAST_TTL   5
#define IP_MULTICAST_IF    6
#define IP_MULTICAST_LOOP  7
#endif /* LWIP_MULTICAST_TX_OPTIONS */

#if LWIP_IGMP
/*
 * Options and types related to multicast membership
 */
#define IP_ADD_MEMBERSHIP  3
#define IP_DROP_MEMBERSHIP 4

typedef struct ip_mreq {
    struct in_addr imr_multiaddr; /* IP multicast address of group */
    struct in_addr imr_interface; /* local IP address of interface */
} ip_mreq;
#endif /* LWIP_IGMP */


#if LWIP_IPV6_MLD
/*
 * Options and types related to IPv6 multicast membership
 */
#define IPV6_JOIN_GROUP      12
#define IPV6_ADD_MEMBERSHIP  IPV6_JOIN_GROUP
#define IPV6_LEAVE_GROUP     13
#define IPV6_DROP_MEMBERSHIP IPV6_LEAVE_GROUP


#endif /* LWIP_IPV6_MLD */

/*
 * The Type of Service provides an indication of the abstract
 * parameters of the quality of service desired.  These parameters are
 * to be used to guide the selection of the actual service parameters
 * when transmitting a datagram through a particular network.  Several
 * networks offer service precedence, which somehow treats high
 * precedence traffic as more important than other traffic (generally
 * by accepting only traffic above a certain precedence at time of high
 * load).  The major choice is a three way tradeoff between low-delay,
 * high-reliability, and high-throughput.
 * The use of the Delay, Throughput, and Reliability indications may
 * increase the cost (in some sense) of the service.  In many networks
 * better performance for one of these parameters is coupled with worse
 * performance on another.  Except for very unusual cases at most two
 * of these three indications should be set.
 */
#define IPTOS_TOS_MASK          0x1E
#define IPTOS_TOS(tos)          ((tos) & IPTOS_TOS_MASK)
#define IPTOS_LOWDELAY          0x10
#define IPTOS_THROUGHPUT        0x08
#define IPTOS_RELIABILITY       0x04
#define IPTOS_LOWCOST           0x02
#define IPTOS_MINCOST           IPTOS_LOWCOST

/*
 * The Network Control precedence designation is intended to be used
 * within a network only.  The actual use and control of that
 * designation is up to each network. The Internetwork Control
 * designation is intended for use by gateway control originators only.
 * If the actual use of these precedence designations is of concern to
 * a particular network, it is the responsibility of that network to
 * control the access to, and use of, those precedence designations.
 */
#define IPTOS_PREC_MASK                 0xe0
#define IPTOS_PREC(tos)                ((tos) & IPTOS_PREC_MASK)
#define IPTOS_PREC_NETCONTROL           0xe0
#define IPTOS_PREC_INTERNETCONTROL      0xc0
#define IPTOS_PREC_CRITIC_ECP           0xa0
#define IPTOS_PREC_FLASHOVERRIDE        0x80
#define IPTOS_PREC_FLASH                0x60
#define IPTOS_PREC_IMMEDIATE            0x40
#define IPTOS_PREC_PRIORITY             0x20
#define IPTOS_PREC_ROUTINE              0x00


/*
 * Commands for ioctlsocket(),  taken from the BSD file fcntl.h.
 * lwip_ioctl only supports FIONREAD and FIONBIO, for now
 *
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 128 bytes.
 */
#if !defined(FIONREAD) || !defined(FIONBIO)
#define IOCPARM_MASK    0x7fU           /* parameters must be < 128 bytes */
#define IOC_VOID        0x20000000UL    /* no parameters */
#define IOC_OUT         0x40000000UL    /* copy out parameters */
#define IOC_IN          0x80000000UL    /* copy in parameters */
#define IOC_INOUT       (IOC_IN|IOC_OUT)
                                        /* 0x20000000 distinguishes new &
                                           old ioctl's */
#define _IO(x,y)        ((long)(IOC_VOID|((x)<<8)|(y)))

#define _IOR(x,y,t)     ((long)(IOC_OUT|((sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y)))

#define _IOW(x,y,t)     ((long)(IOC_IN|((sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y)))
#endif /* !defined(FIONREAD) || !defined(FIONBIO) */

#ifndef FIONREAD
#define FIONREAD    _IOR('f', 127, unsigned long) /* get # bytes to read */
#endif
#ifndef FIONBIO
#define FIONBIO     _IOW('f', 126, unsigned long) /* set/clear non-blocking i/o */
#endif

/* Socket I/O Controls: unimplemented */
#ifndef SIOCSHIWAT
#define SIOCSHIWAT  _IOW('s',  0, unsigned long)  /* set high watermark */
#define SIOCGHIWAT  _IOR('s',  1, unsigned long)  /* get high watermark */
#define SIOCSLOWAT  _IOW('s',  2, unsigned long)  /* set low watermark */
#define SIOCGLOWAT  _IOR('s',  3, unsigned long)  /* get low watermark */
#define SIOCATMARK  _IOR('s',  7, unsigned long)  /* at oob mark? */
#endif

/* commands for fnctl */
#ifndef F_GETFL
#define F_GETFL 3
#endif
#ifndef F_SETFL
#define F_SETFL 4
#endif

/* File status flags and file access modes for fnctl,
   these are bits in an int. */
#ifndef O_NONBLOCK
#define O_NONBLOCK  1 /* nonblocking I/O */
#endif
#ifndef O_NDELAY
#define O_NDELAY    O_NONBLOCK /* same as O_NONBLOCK, for compatibility */
#endif
#ifndef O_RDONLY
#define O_RDONLY    2
#endif
#ifndef O_WRONLY
#define O_WRONLY    4
#endif
#ifndef O_RDWR
#define O_RDWR      (O_RDONLY|O_WRONLY)
#endif

static int
lwip_sockopt_to_ipopt(int optname)
{
	/* Map SO_* values to our internal SOF_* values
	* We should not rely on #defines in socket.h
	* being in sync with ip.h.
	*/
	switch (optname) {
	case SO_BROADCAST:
		return SOF_BROADCAST;
	case SO_KEEPALIVE:
		return SOF_KEEPALIVE;
	case SO_REUSEADDR:
		return SOF_REUSEADDR;
	default:
		LWIP_ASSERT("Unknown socket option", 0);
		return 0;
	}
}

#define MAX_SOCK_NUM LWIP_NUM_SOCKETS
extern LUAT_WEAK void DBG_Printf(const char* format, ...);
#ifdef LUAT_LOG_NO_NEWLINE
#define NET_DBG(x,y...) DBG_Printf("%s %d:"x, __FUNCTION__,__LINE__,##y)
#else
#define NET_DBG(x,y...) DBG_Printf("%s %d:"x"\r\n", __FUNCTION__,__LINE__,##y)
#endif
enum
{
	EV_LWIP_EVENT_START = USER_EVENT_ID_START + 0x2000000,
	EV_LWIP_SOCKET_TX,
	EV_LWIP_NETIF_INPUT,
	EV_LWIP_TCP_TIMER,
	EV_LWIP_COMMON_TIMER,
	EV_LWIP_SOCKET_RX_DONE,
	EV_LWIP_SOCKET_CREATE,
	EV_LWIP_SOCKET_CONNECT,
	EV_LWIP_SOCKET_DNS,
	EV_LWIP_SOCKET_LISTEN,
	EV_LWIP_SOCKET_ACCPET,
	EV_LWIP_SOCKET_CLOSE,
	EV_LWIP_NETIF_LINK_STATE,
	EV_LWIP_DHCP_TIMER,
	EV_LWIP_FAST_TIMER,
	EV_LWIP_NETIF_SET_IP,
	EV_LWIP_NETIF_IPV6_BY_MAC,
};
extern u32_t tcp_ticks;
extern struct tcp_pcb *tcp_active_pcbs;
extern struct tcp_pcb *tcp_tw_pcbs;
#define SOCKET_LOCK(ID)		platform_lock_mutex(prvlwip.socket[ID].mutex)
#define SOCKET_UNLOCK(ID)	platform_unlock_mutex(prvlwip.socket[ID].mutex)


typedef struct
{
	llist_head node;
	uint64_t tag;	//考虑到socket复用的问题，必须有tag来做比对
	luat_ip_addr_t ip;
	uint8_t *data;
	uint32_t read_pos;
	uint16_t len;
	uint16_t port;
	uint8_t is_sending;
	uint8_t is_need_ack;
}socket_data_t;

typedef struct
{
	dns_client_t dns_client;
	socket_ctrl_t socket[MAX_SOCK_NUM];
	struct netif *lwip_netif[NW_ADAPTER_INDEX_LWIP_NETIF_QTY];
	uint64_t last_sleep_ms;
	uint64_t socket_tag;
	CBFuncEx_t socket_cb[NW_ADAPTER_INDEX_LWIP_NETIF_QTY];
	void *user_data[NW_ADAPTER_INDEX_LWIP_NETIF_QTY];
	void *task_handle;
	struct udp_pcb *dns_udp;
	HANDLE socket_mutex;
	HANDLE tcp_timer;//tcp_tmr
	HANDLE common_timer;//ip_reass_tmr,etharp_tmr,dns_tmr,nd6_tmr,ip6_reass_tmr
	HANDLE fast_timer;//igmp_tmr,mld6_tmr,autoip_tmr
	HANDLE dhcp_timer;//dhcp_fine_tmr,dhcp6_tmr
	uint8_t netif_network_ready[NW_ADAPTER_INDEX_LWIP_NETIF_QTY];
	uint8_t tcpip_tcp_timer_active;
	uint8_t common_timer_active;
	uint8_t dhcp_timer_active;
	uint8_t fast_timer_active;
	uint8_t dhcp_check_cnt;
	uint8_t next_socket_index;
}net_lwip_ctrl_struct;

static net_lwip_ctrl_struct prvlwip;
static void net_lwip_check_network_ready(uint8_t adapter_index);
static void net_lwip_task(void *param);
static void net_lwip_dhcp_done_cb(struct netif *netif)
{
	net_lwip_check_network_ready((uint8_t)netif->dhcp_done_arg);
}
static int net_lwip_del_data_cache(void *p, void *u)
{
	socket_data_t *pdata = (socket_data_t *)p;
	free(pdata->data);
	return LIST_DEL;
}

static int net_lwip_next_data_cache(void *p, void *u)
{
	socket_ctrl_t *socket = (socket_ctrl_t *)u;
	socket_data_t *pdata = (socket_data_t *)p;
	if (socket->tag != pdata->tag)
	{
		NET_DBG("tag error");
		free(pdata->data);
		return LIST_DEL;
	}
	return LIST_FIND;
}


static socket_data_t * net_lwip_create_data_node(uint8_t socket_id, uint8_t *data, uint32_t len, luat_ip_addr_t *remote_ip, uint16_t remote_port)
{
	socket_data_t *p = (socket_data_t *)malloc(sizeof(socket_data_t));
	if (p)
	{
		memset(p, 0, sizeof(socket_data_t));
		p->len = len;
		p->port = remote_port;
		if (remote_ip)
		{
			p->ip = *remote_ip;
		}
		else
		{
			p->ip.type = 0xff;
		}
		p->tag = prvlwip.socket[socket_id].tag;
		if (data && len)
		{
			p->data = malloc(len);
			if (p->data)
			{
				memcpy(p->data, data, len);
			}
			else
			{
				free(p);
				return NULL;
			}
		}
	}
	return p;
}

static LUAT_RT_RET_TYPE net_lwip_timer_cb(LUAT_RT_CB_PARAM)
{
	luat_send_event_to_task(prvlwip.task_handle, (uint32_t)param, 0, 0, 0);
	return LUAT_RT_RET;
}

static void net_lwip_callback_to_nw_task(uint8_t adapter_index, uint32_t event_id, uint32_t param1, uint32_t param2, uint32_t param3)
{
	luat_network_cb_param_t param = {.tag = 0, .param = prvlwip.user_data[adapter_index]};
	OS_EVENT event = { .ID = event_id, .Param1 = param1, .Param2 = param2, .Param3 = param3};
	if (event_id > EV_NW_DNS_RESULT)
	{
		event.Param3 = prvlwip.socket[param1].param;
		param.tag = prvlwip.socket[param1].tag;
	}
	prvlwip.socket_cb[adapter_index](&event, &param);
}


static err_t net_lwip_tcp_connected_cb(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	int socket_id = ((uint32_t)arg) & 0x0000ffff;
	uint8_t adapter_index = ((uint32_t)arg) >> 16;
	net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_CONNECT_OK, socket_id, 0, 0);
	return ERR_OK;
}

static err_t net_lwip_tcp_accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	int socket_id = ((uint32_t)arg) & 0x0000ffff;
	uint8_t adapter_index = ((uint32_t)arg) >> 16;
	return ERR_OK;
}

static int net_lwip_rx_data(int socket_id, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	int is_mem_err = 0;
	SOCKET_LOCK(socket_id);
	socket_data_t *data_p = net_lwip_create_data_node(socket_id, NULL, 0, addr, port);
	if (data_p)
	{
		data_p->data = malloc(p->tot_len);
		if (data_p->data)
		{
			data_p->len = pbuf_copy_partial(p, data_p->data, p->tot_len, 0);
			llist_add_tail(&data_p->node, &prvlwip.socket[socket_id].rx_head);
			prvlwip.socket[socket_id].rx_wait_size += p->tot_len;
		}
		else
		{
			free(data_p);
			is_mem_err = 1;
		}
	}
	else
	{
		is_mem_err = 1;
	}
	SOCKET_UNLOCK(socket_id);
	pbuf_free(p);
	return is_mem_err;
}


static err_t net_lwip_tcp_recv_cb(void *arg, struct tcp_pcb *tpcb,
                             struct pbuf *p, err_t err)
{
	int socket_id = ((uint32_t)arg) & 0x0000ffff;
	uint8_t adapter_index = ((uint32_t)arg) >> 16;
	uint16_t len;

	if (p)
	{
		len = p->tot_len;
		if (net_lwip_rx_data(socket_id, p, NULL, 0))
		{
			NET_DBG("no memory!");
			net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_ERROR, socket_id, 0, 0);
		}
		else
		{
			net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_RX_NEW, socket_id, len, 0);
		}

	}
	else if (err == ERR_OK)
	{
		net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_REMOTE_CLOSE, socket_id, 0, 0);
	}
	else
	{
		net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_ERROR, socket_id, 0, 0);
	}
	return ERR_OK;
}

static err_t net_lwip_tcp_sent_cb(void *arg, struct tcp_pcb *tpcb,
                              u16_t len)
{
	int socket_id = ((uint32_t)arg) & 0x0000ffff;
	uint8_t adapter_index = ((uint32_t)arg) >> 16;
	volatile uint16_t check_len = 0;
	volatile uint32_t rest_len;
	socket_data_t *p;
	SOCKET_LOCK(socket_id);
	while(check_len < len)
	{
		if (llist_empty(&prvlwip.socket[socket_id].wait_ack_head))
		{
			NET_DBG("!");
			goto SOCEKT_ERROR;
		}
		p = (socket_data_t *)prvlwip.socket[socket_id].wait_ack_head.next;
		rest_len = p->len - p->read_pos;
		if ((len - check_len) >= rest_len)
		{
			check_len += rest_len;
			NET_DBG("socket %d, %ubytes ack", socket_id, p->len);
			llist_del(&p->node);
			free(p->data);
			free(p);
			check_len += rest_len;
		}
		else
		{
			p->read_pos += (len - check_len);
			check_len = len;
			NET_DBG("socket %d, all %ubytes ack %ubytes ", socket_id, p->len, p->read_pos);
		}
	}
	while (!llist_empty(&prvlwip.socket[socket_id].tx_head))
	{
		p = llist_traversal(&prvlwip.socket[socket_id].tx_head, net_lwip_next_data_cache, &prvlwip.socket[socket_id]);
		if (p)
		{
			if (ERR_OK == tcp_write(prvlwip.socket[socket_id].pcb.tcp, p->data, p->len, 0))
			{
				llist_del(&p->node);
				llist_add_tail(&p->node, &prvlwip.socket[socket_id].wait_ack_head);
			}
			else
			{
				NET_DBG("tcp buf is full, wait ack and send again");
				break;
			}
		}
	}

	SOCKET_UNLOCK(socket_id);
	tcp_output(prvlwip.socket[socket_id].pcb.tcp);
	net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_TX_OK, socket_id, len, 0);
	return ERR_OK;
SOCEKT_ERROR:
	SOCKET_UNLOCK(socket_id);
	net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_ERROR, socket_id, 0, 0);
	return ERR_OK;
}

static err_t net_lwip_tcp_err_cb(void *arg, err_t err)
{
	int socket_id = ((uint32_t)arg) & 0x0000ffff;
	uint8_t adapter_index = ((uint32_t)arg) >> 16;
	prvlwip.socket[socket_id].pcb.ip = NULL;
	net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_ERROR, socket_id, 0, 0);
}

static err_t net_lwip_udp_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p,
    const ip_addr_t *addr, u16_t port)
{
	int socket_id = ((uint32_t)arg) & 0x0000ffff;
	uint8_t adapter_index = ((uint32_t)arg) >> 16;
	uint16_t len;
	if (p)
	{
		len = p->tot_len;
		if (net_lwip_rx_data(socket_id, p, addr, port))
		{
			NET_DBG("no memory!");
			net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_ERROR, socket_id, 0, 0);
		}
		else
		{
			net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_RX_NEW, socket_id, len, 0);
		}
	}
	return ERR_OK;
}

static int32_t net_lwip_dns_check_result(void *data, void *param)
{
	luat_dns_require_t *require = (luat_dns_require_t *)data;
	if (require->result != 0)
	{
		free(require->uri.Data);
		require->uri.Data = NULL;
		if (require->result > 0)
		{
			luat_dns_ip_result *ip_result = zalloc(sizeof(luat_dns_ip_result) * require->result);
			int i;
			for(i = 0; i < require->result; i++)
			{
				ip_result[i] = require->ip_result[i];
			}
			net_lwip_callback_to_nw_task(require->adapter_index, EV_NW_DNS_RESULT, require->result, ip_result, require->param);
		}
		else
		{
			net_lwip_callback_to_nw_task(require->adapter_index, EV_NW_DNS_RESULT, 0, 0, require->param);
		}

		return LIST_DEL;
	}
	else
	{
		return LIST_PASS;
	}
}

static err_t net_lwip_dns_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p,
    const ip_addr_t *addr, u16_t port)
{
	Buffer_Struct msg_buf;
	Buffer_Struct tx_msg_buf = {0,0,0};
	struct pbuf *out_p;
	int i;
	if (p)
	{
		msg_buf.Data = p->payload;
		msg_buf.MaxLen = p->len;
		dns_run(&prvlwip.dns_client, &msg_buf, NULL, &i);
		llist_traversal(&prvlwip.dns_client.require_head, net_lwip_dns_check_result, NULL);
		{
			dns_run(&prvlwip.dns_client, NULL, &tx_msg_buf, &i);
			if (tx_msg_buf.Pos)
			{
				out_p = pbuf_alloc(PBUF_RAW, tx_msg_buf.Pos, PBUF_ROM);
				if (out_p)
				{
					out_p->payload = tx_msg_buf.Data;
					udp_sendto(prvlwip.dns_udp, out_p, &prvlwip.dns_client.dns_server[i], DNS_SERVER_PORT);
					pbuf_free(out_p);
				}
				OS_DeInitBuffer(&tx_msg_buf);
				llist_traversal(&prvlwip.dns_client.require_head, net_lwip_dns_check_result, NULL);
			}
		}
		pbuf_free(p);
	}
	return ERR_OK;
}




static void net_lwip_dns_tx_next(Buffer_Struct *tx_msg_buf)
{
	int i;
	struct pbuf *p;
	dns_run(&prvlwip.dns_client, NULL, tx_msg_buf, &i);
	if (tx_msg_buf->Pos)
	{
		p = pbuf_alloc(PBUF_RAW, tx_msg_buf->Pos, PBUF_ROM);
		if (p)
		{
			p->payload = tx_msg_buf->Data;
			udp_sendto(prvlwip.dns_udp, p, &prvlwip.dns_client.dns_server[i], DNS_SERVER_PORT);
			pbuf_free(p);
		}
		OS_DeInitBuffer(tx_msg_buf);
		llist_traversal(&prvlwip.dns_client.require_head, net_lwip_dns_check_result, NULL);
	}
}

uint32_t net_lwip_rand()
{
	PV_Union uPV;
	luat_crypto_trng(uPV.u8, 4);
	return uPV.u32;
}

void net_lwip_init(void)
{
	luat_thread_t thread;
	uint8_t i;
	for(i = 0; i < MAX_SOCK_NUM; i++)
	{
		INIT_LLIST_HEAD(&prvlwip.socket[i].wait_ack_head);
		INIT_LLIST_HEAD(&prvlwip.socket[i].tx_head);
		INIT_LLIST_HEAD(&prvlwip.socket[i].rx_head);
	}
	prvlwip.socket_mutex = platform_create_mutex();
	prvlwip.tcp_timer = platform_create_timer(net_lwip_timer_cb, (void *)EV_LWIP_TCP_TIMER, 0);
	prvlwip.common_timer = platform_create_timer(net_lwip_timer_cb, (void *)EV_LWIP_COMMON_TIMER, 0);
	prvlwip.fast_timer = platform_create_timer(net_lwip_timer_cb, (void *)EV_LWIP_FAST_TIMER, 0);
	prvlwip.dhcp_timer = platform_create_timer(net_lwip_timer_cb, (void *)EV_LWIP_DHCP_TIMER, 0);
	tcp_ticks = luat_mcu_tick64_ms() / TCP_SLOW_INTERVAL;
	prvlwip.last_sleep_ms = luat_mcu_tick64_ms();
	thread.task_fun = net_lwip_task;
	thread.name = "lwip";
	thread.stack_size = 16 * 1024;
	thread.priority = 60;
	platform_create_task(&thread);
	prvlwip.task_handle = thread.handle;
	lwip_init();
	platform_start_timer(prvlwip.common_timer, 1000, 1);

	prvlwip.dns_udp = udp_new();
	prvlwip.dns_udp->local_port = 54;
	dns_init_client(&prvlwip.dns_client);
}



void tcp_timer_needed(void)
{
  if (!prvlwip.tcpip_tcp_timer_active && (tcp_active_pcbs || tcp_tw_pcbs)) {
	  prvlwip.tcpip_tcp_timer_active = 1;
	  platform_start_timer(prvlwip.tcp_timer, TCP_TMR_INTERVAL, 1);
	  NET_DBG("tcp timer start");
  }
}


u32_t sys_now(void)
{
	return (u32_t)luat_mcu_tick64_ms();
}

static void net_lwip_task(void *param)
{
	OS_EVENT event;
	Buffer_Struct tx_msg_buf = {0,0,0};
	HANDLE cur_task = luat_get_current_task();
	struct netif *netif;
	struct dhcp *dhcp;
	socket_data_t *p;
	luat_ip_addr_t *p_ip;
	struct pbuf *out_p;
	int error, i;
	PV_Union uPV;
	uint8_t active_flag;
	uint8_t socket_id;
	uint8_t adapter_index;
	while(1)
	{

		if (luat_wait_event_from_task(cur_task, 0, &event, NULL, 0) != ERROR_NONE)
		{
			continue;
		}
		if (!prvlwip.tcpip_tcp_timer_active)
		{
			if ((luat_mcu_tick64_ms() - prvlwip.last_sleep_ms) >= TCP_SLOW_INTERVAL)
			{
				tcp_ticks += (luat_mcu_tick64_ms() - prvlwip.last_sleep_ms) / TCP_SLOW_INTERVAL;
				prvlwip.last_sleep_ms = luat_mcu_tick64_ms();
				NET_DBG("tcp ticks add to %u", tcp_ticks);
			}
		}
		else
		{
			prvlwip.last_sleep_ms = luat_mcu_tick64_ms();
		}
		socket_id = event.Param1;
		adapter_index = event.Param3;
		switch(event.ID)
		{
		case EV_LWIP_SOCKET_TX:

			SOCKET_LOCK(socket_id);
			if (prvlwip.socket[socket_id].in_use && prvlwip.socket[socket_id].pcb.ip)
			{
				if (prvlwip.socket[socket_id].is_tcp)
				{
					while (!llist_empty(&prvlwip.socket[socket_id].tx_head))
					{
						p = llist_traversal(&prvlwip.socket[socket_id].tx_head, net_lwip_next_data_cache, &prvlwip.socket[socket_id]);
						if (p->len <= tcp_sndbuf(prvlwip.socket[socket_id].pcb.tcp))
						{
							if (ERR_OK == tcp_write(prvlwip.socket[socket_id].pcb.tcp, p->data, p->len, 0))
							{
								llist_del(&p->node);
								llist_add_tail(&p->node, &prvlwip.socket[socket_id].wait_ack_head);
							}
							else
							{
								NET_DBG("tcp buf is full, wait ack and send again");
								break;
							}
						}
					}

					SOCKET_UNLOCK(socket_id);
					tcp_output(prvlwip.socket[socket_id].pcb.tcp);
				}
				else
				{
					p = llist_traversal(&prvlwip.socket[socket_id].tx_head, net_lwip_next_data_cache, &prvlwip.socket[socket_id]);
					if (p)
					{
						llist_del(&p->node);
					}
					SOCKET_UNLOCK(socket_id);
					if (p)
					{

						out_p = pbuf_alloc(PBUF_RAW, p->len, PBUF_ROM);
						if (out_p)
						{
							out_p->payload = p->data;
							udp_sendto(prvlwip.dns_udp, out_p, &p->ip, p->port);
							pbuf_free(out_p);
						}
						else
						{
							NET_DBG("mem err send fail");
						}
						uint32_t len = p->len;
						free(p->data);
						free(p);
						net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_TX_OK, socket_id, len, 0);

					}
				}
			}
			else
			{
				NET_DBG("socket %d no in use! %x", socket_id, prvlwip.socket[socket_id].pcb.ip);
				SOCKET_UNLOCK(socket_id);
				net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_ERROR, socket_id, 0, 0);
			}


			break;
		case EV_LWIP_NETIF_INPUT:
			netif = (struct netif *)event.Param3;
			if(netif->input((struct pbuf *)event.Param1, netif) != ERR_OK)
			{
				pbuf_free((struct pbuf *)event.Param1);
			}
			break;
		case EV_LWIP_TCP_TIMER:
			tcp_tmr();
			if (tcp_active_pcbs || tcp_tw_pcbs)
			{

			}
			else
			{
				prvlwip.tcpip_tcp_timer_active = 0;
				platform_stop_timer(prvlwip.tcp_timer);
				NET_DBG("tcp timer stop");
			}
			break;
		case EV_LWIP_COMMON_TIMER:
#ifdef LUAT_USE_DNS
			net_lwip_dns_tx_next(&tx_msg_buf);
#endif
#if IP_REASSEMBLY
			ip_reass_tmr();
#endif
#if LWIP_ARP
			etharp_tmr();
#endif
#if LWIP_DNS
			dns_tmr();
#endif
#if LWIP_IPV6
			nd6_tmr();
#endif
#if LWIP_IPV6_REASS
			ip6_reass_tmr();
#endif
#if LWIP_DHCP
			prvlwip.dhcp_check_cnt++;
			if (prvlwip.dhcp_check_cnt >= DHCP_COARSE_TIMER_SECS)
			{
				prvlwip.dhcp_check_cnt = 0;
				dhcp_coarse_tmr();
				if (!prvlwip.dhcp_timer_active)
				{
					prvlwip.dhcp_timer_active = 1;
					platform_start_timer(prvlwip.dhcp_timer, DHCP_FINE_TIMER_MSECS, 1);
					NET_DBG("dhcp timer start");
				}
			}
#endif
			break;
		case EV_LWIP_SOCKET_RX_DONE:
			if (!prvlwip.socket[socket_id].in_use || !prvlwip.socket[socket_id].pcb.ip || !prvlwip.socket[socket_id].is_tcp)
			{
				NET_DBG("error socket %d state %d,%x,%d", socket_id, prvlwip.socket[socket_id].in_use, prvlwip.socket[socket_id].pcb.ip, prvlwip.socket[socket_id].is_tcp);
				break;
			}
			tcp_recved(prvlwip.socket[socket_id].pcb.tcp, event.Param2);
			break;
		case EV_LWIP_SOCKET_CREATE:
			uPV.u16[0] = socket_id;
			uPV.u16[1] = adapter_index;
			if (prvlwip.socket[socket_id].is_tcp)
			{
				prvlwip.socket[socket_id].pcb.tcp = tcp_new();
				if (prvlwip.socket[socket_id].pcb.tcp)
				{
					prvlwip.socket[socket_id].pcb.tcp->netif_idx = netif_get_index(prvlwip.lwip_netif[adapter_index]);
					prvlwip.socket[socket_id].rx_wait_size = 0;
					prvlwip.socket[socket_id].tx_wait_size = 0;
					prvlwip.socket[socket_id].pcb.tcp->callback_arg = uPV.p;
					prvlwip.socket[socket_id].pcb.tcp->recv = net_lwip_tcp_recv_cb;
					prvlwip.socket[socket_id].pcb.tcp->sent = net_lwip_tcp_sent_cb;
					prvlwip.socket[socket_id].pcb.tcp->errf = net_lwip_tcp_err_cb;
					prvlwip.socket[socket_id].pcb.tcp->so_options |= SOF_KEEPALIVE|SOF_REUSEADDR;
//					tcp_set_flags(prvlwip.socket[socket_id].pcb.tcp, TCP_NODELAY);
					if (adapter_index == NW_ADAPTER_INDEX_LWIP_GPRS)
					{
						prvlwip.socket[socket_id].pcb.tcp->more_delay = 2;
					}

				}
				else
				{
					NET_DBG("tcp pcb full!");
				}
			}
			else
			{
				prvlwip.socket[socket_id].pcb.udp = udp_new();
				if (prvlwip.socket[socket_id].pcb.udp)
				{
					prvlwip.socket[socket_id].pcb.udp->netif_idx = netif_get_index(prvlwip.lwip_netif[adapter_index]);
					prvlwip.socket[socket_id].pcb.udp->recv_arg = uPV.p;
					prvlwip.socket[socket_id].pcb.udp->recv = net_lwip_udp_recv_cb;
					prvlwip.socket[socket_id].pcb.udp->so_options |= SOF_BROADCAST|SOF_REUSEADDR;
				}
				else
				{
					NET_DBG("udp pcb full!");
				}
			}

			break;
		case EV_LWIP_SOCKET_CONNECT:
			if (!prvlwip.socket[socket_id].in_use || !prvlwip.socket[socket_id].pcb.ip)
			{
				NET_DBG("socket %d cannot use! %d,%x", socket_id, prvlwip.socket[socket_id].in_use, prvlwip.socket[socket_id].pcb.ip);
				net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_ERROR, socket_id, 0, 0);
				break;
			}

			if (prvlwip.socket[socket_id].is_tcp)
			{

				tcp_bind(prvlwip.socket[socket_id].pcb.tcp, NULL, prvlwip.socket[socket_id].local_port);
				error = tcp_connect(prvlwip.socket[socket_id].pcb.tcp, (luat_ip_addr_t *)event.Param2, prvlwip.socket[socket_id].remote_port, net_lwip_tcp_connected_cb);
				if (error)
				{
					net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_ERROR, socket_id, 0, 0);
				}
			}
			else
			{
				udp_bind(prvlwip.socket[socket_id].pcb.udp, NULL, prvlwip.socket[socket_id].local_port);
				error = udp_connect(prvlwip.socket[socket_id].pcb.udp, (luat_ip_addr_t *)event.Param2, prvlwip.socket[socket_id].remote_port);
				if (error)
				{
					net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_ERROR, socket_id, 0, 0);
				}
				else
				{
					net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_CONNECT_OK, socket_id, 0, 0);
				}

			}
			break;
		case EV_LWIP_SOCKET_DNS:
			dns_require_ex(&prvlwip.dns_client, event.Param1, event.Param2, event.Param3);
			net_lwip_dns_tx_next(&tx_msg_buf);
			break;
		case EV_LWIP_SOCKET_LISTEN:

			break;
		case EV_LWIP_SOCKET_ACCPET:

			break;
		case EV_LWIP_SOCKET_CLOSE:
			if (!prvlwip.socket[socket_id].in_use)
			{
				NET_DBG("socket %d no in use!,%x", socket_id);
				break;
			}
			if (prvlwip.socket[socket_id].pcb.ip)
			{
				if (prvlwip.socket[socket_id].is_tcp)
				{
					error = tcp_close(prvlwip.socket[socket_id].pcb.tcp);
					if (error)
					{
						NET_DBG("tcp close error %d", error);
					}
				}
				else
				{
					udp_remove(prvlwip.socket[socket_id].pcb.udp);
				}
			}
			SOCKET_LOCK(socket_id);
			OS_LOCK;
			prvlwip.socket[socket_id].state = 0;
			prvlwip.socket[socket_id].in_use = 0;
			prvlwip.socket[socket_id].tag = 0;
			prvlwip.socket[socket_id].param = NULL;
			prvlwip.socket[socket_id].rx_wait_size = 0;
			prvlwip.socket[socket_id].tx_wait_size = 0;
			llist_traversal(&prvlwip.socket[socket_id].wait_ack_head, net_lwip_del_data_cache, NULL);
			llist_traversal(&prvlwip.socket[socket_id].tx_head, net_lwip_del_data_cache, NULL);
			llist_traversal(&prvlwip.socket[socket_id].rx_head, net_lwip_del_data_cache, NULL);
			OS_UNLOCK;
			SOCKET_UNLOCK(socket_id);

			if (event.Param2)
			{
				net_lwip_callback_to_nw_task(adapter_index, EV_NW_SOCKET_CLOSE_OK, socket_id, 0, 0);
			}

			break;
		case EV_LWIP_DHCP_TIMER:
#if LWIP_DHCP
			dhcp_fine_tmr();
			active_flag = 0;
			NETIF_FOREACH(netif)
			{
				dhcp = netif_dhcp_data(netif);
				/* only act on DHCP configured interfaces */
				if (dhcp && dhcp->request_timeout && (dhcp->state != DHCP_STATE_BOUND))
				{
					active_flag = 1;
					break;
				}
			}
			if (!active_flag)
			{
				NET_DBG("stop dhcp timer!");
				prvlwip.dhcp_timer_active = 0;
				platform_stop_timer(prvlwip.dhcp_timer);
			}
#endif
			break;
		case EV_LWIP_FAST_TIMER:
#if LWIP_AUTOIP
			autoip_tmr();
#endif
#if LWIP_IGMP
			igmp_tmr();
#endif
#if LWIP_IPV6_MLD
			mld6_tmr();
#endif
			active_flag = 0;
			NETIF_FOREACH(netif)
			{

				if (
#if LWIP_IPV6_MLD
						netif_mld6_data(netif)
#endif
#if LWIP_IGMP
						|| netif_igmp_data(netif)
#endif
#if LWIP_AUTOIP
						|| netif_autoip_data(netif)
#endif
					)
				{
					active_flag = 1;
					break;
				}

			}
			if (!active_flag)
			{
				NET_DBG("stop fast timer!");
				prvlwip.fast_timer_active = 0;
				platform_stop_timer(prvlwip.fast_timer);
			}
			break;

		case EV_LWIP_NETIF_SET_IP:
			netif = prvlwip.lwip_netif[event.Param3];
			p_ip = (ip_addr_t *)event.Param1;
			if ((p_ip[0].type != IPADDR_TYPE_ANY) && p_ip[0].u_addr.ip4.addr)
			{
				dhcp_release_and_stop(netif);
			}
			netif_set_addr(netif, &p_ip[0].u_addr.ip4, &p_ip[1].u_addr.ip4, &p_ip[2].u_addr.ip4);
			netif_ip6_addr_set(netif, 1, &p_ip[3]);
			if (event.Param2)
			{
				netif_ip6_addr_set_state(netif, 1, IP6_ADDR_VALID);
			}
			else
			{
				netif_ip6_addr_set_state(netif, 1, IP6_ADDR_INVALID);
			}
			if (!prvlwip.fast_timer_active)
			{
				NET_DBG("start fast timer!");
				prvlwip.fast_timer_active = 1;
				platform_start_timer(prvlwip.fast_timer, 100, 1);
			}
			free(p_ip);
			break;
		case EV_LWIP_NETIF_LINK_STATE:
			NET_DBG("%d,%d,%d", event.Param3, prvlwip.netif_network_ready[event.Param3], event.Param1);
			if (prvlwip.netif_network_ready[event.Param3] != event.Param1)
			{
				if (event.Param1)
				{
					netif_set_link_up(prvlwip.lwip_netif[event.Param3]);
				}
				else
				{
					netif_set_link_down(prvlwip.lwip_netif[event.Param3]);
				}
			}
			net_lwip_check_network_ready(event.Param3);
			break;
		case EV_LWIP_NETIF_IPV6_BY_MAC:
			netif_create_ip6_linklocal_address(prvlwip.lwip_netif[event.Param3], 1);
			break;
		default:
			NET_DBG("unknow event %x,%x", event.ID, event.Param1);
			break;
		}
	}

}

static void net_lwip_check_network_ready(uint8_t adapter_index)
{
	int i;
	char ip_string[64];
	struct netif *netif = prvlwip.lwip_netif[adapter_index];
	uint8_t active_flag = netif_is_flag_set(netif, NETIF_FLAG_LINK_UP);
	uint8_t ip_ready = 0;
	if (!active_flag)
	{
		dhcp_release_and_stop(netif);
	}
#if LWIP_IPV4
	if (netif->ip_addr.u_addr.ip4.addr)
	{
		ip_ready = 1;
	}
#endif
#if LWIP_IPV6
	for (i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
	{
	    if (ip6_addr_isinvalid(netif_ip6_addr_state(netif, i)))
	    {
	    	ip_ready = 1;
	    	break;
	    }
	}
#endif
	if (!ip_ready)
	{
#if LWIP_DHCP
		if (netif->flags & NETIF_FLAG_ETHARP)
		{
			dhcp_start(netif);
			if (!prvlwip.dhcp_timer_active)
			{
				prvlwip.dhcp_timer_active = 1;
				platform_start_timer(prvlwip.dhcp_timer, 500, 1);
			}
		}
#endif
		active_flag = 0;
	}
	if (prvlwip.netif_network_ready[adapter_index] != active_flag)
	{
		prvlwip.netif_network_ready[adapter_index] = active_flag;
		if (!active_flag)
		{
			dns_clear(&prvlwip.dns_client);
			prvlwip.dns_client.is_run = 0;
			NET_DBG("network not ready");
			net_lwip_callback_to_nw_task(adapter_index, EV_NW_STATE, 0, 0, adapter_index);
		}
		else
		{
			NET_DBG("network ready");
			for(i = 0; i < MAX_DNS_SERVER; i++)
			{
				if (prvlwip.dns_client.dns_server[i].type != 0xff)
				{
					NET_DBG("DNS%d:%s",i, ipaddr_ntoa_r(&prvlwip.dns_client.dns_server[i], ip_string, sizeof(ip_string)));
				}
			}
			net_lwip_callback_to_nw_task(adapter_index, EV_NW_STATE, 0, 1, adapter_index);
		}

	}
}

static int net_lwip_check_socket(void *user_data, int socket_id, uint64_t tag)
{
	if ((uint32_t)user_data >= NW_ADAPTER_INDEX_LWIP_NETIF_QTY) return -1;
	if (socket_id >= MAX_SOCK_NUM) return -1;
	if (prvlwip.socket[socket_id].tag != tag) return -1;
	if (!prvlwip.socket[socket_id].in_use || prvlwip.socket[socket_id].state) return -1;
	return 0;
}

static int net_lwip_socket_check(int socket_id, uint64_t tag, void *user_data)
{
	return net_lwip_check_socket(user_data, socket_id, tag);
}


static uint8_t net_lwip_check_ready(void *user_data)
{
	if ((uint32_t)user_data >= NW_ADAPTER_INDEX_LWIP_NETIF_QTY) return 0;
	NET_DBG("%d", user_data);
	return (prvlwip.netif_network_ready[(uint32_t)user_data]);
}

static int net_lwip_create_soceket(uint8_t is_tcp, uint64_t *tag, void *param, uint8_t is_ipv6, void *user_data)
{
	uint8_t index = (uint32_t)user_data;
	if (index >= NW_ADAPTER_INDEX_LWIP_NETIF_QTY) return -1;
	int i, socket_id;
	socket_id = -1;
	OS_LOCK;
	if (!prvlwip.socket[prvlwip.next_socket_index].in_use)
	{
		socket_id = prvlwip.next_socket_index;
		prvlwip.next_socket_index++;
	}
	else
	{
		for (i = 0; i < MAX_SOCK_NUM; i++)
		{
			if (!prvlwip.socket[i].in_use)
			{
				socket_id = i;
				prvlwip.next_socket_index = i + 1;
				break;
			}
		}
	}
	if (prvlwip.next_socket_index >= MAX_SOCK_NUM)
	{
		prvlwip.next_socket_index = 0;
	}

	if (socket_id >= 0)
	{
		LWIP_ASSERT("socket must free before create", !prvlwip.socket[socket_id].pcb.ip);
		prvlwip.socket_tag++;
		*tag = prvlwip.socket_tag;
		prvlwip.socket[socket_id].in_use = 1;
		prvlwip.socket[socket_id].tag = *tag;
		prvlwip.socket[socket_id].param = param;
		prvlwip.socket[socket_id].is_tcp = is_tcp;
		llist_traversal(&prvlwip.socket[socket_id].wait_ack_head, net_lwip_del_data_cache, NULL);
		llist_traversal(&prvlwip.socket[socket_id].tx_head, net_lwip_del_data_cache, NULL);
		llist_traversal(&prvlwip.socket[socket_id].rx_head, net_lwip_del_data_cache, NULL);
		OS_UNLOCK;
		platform_send_event(prvlwip.task_handle, EV_LWIP_SOCKET_CREATE, socket_id, 0, user_data);
	}
	else
	{
		OS_UNLOCK;
	}

	return socket_id;
}

//作为client绑定一个port，并连接remote_ip和remote_port对应的server
static int net_lwip_socket_connect(int socket_id, uint64_t tag,  uint16_t local_port, luat_ip_addr_t *remote_ip, uint16_t remote_port, void *user_data)
{
	int result = net_lwip_check_socket(user_data, socket_id, tag);
	if (result) return result;

	prvlwip.socket[socket_id].local_port = local_port;
	prvlwip.socket[socket_id].remote_port = remote_port;
	platform_send_event(prvlwip.task_handle, EV_LWIP_SOCKET_CONNECT, socket_id, remote_ip, user_data);
	return 0;
}
//作为server绑定一个port，开始监听
static int net_lwip_socket_listen(int socket_id, uint64_t tag,  uint16_t local_port, void *user_data)
{
	int result = net_lwip_check_socket(user_data, socket_id, tag);
	if (result) return result;
	platform_send_event(prvlwip.task_handle, EV_LWIP_SOCKET_LISTEN, socket_id, local_port, user_data);
	return 0;
}
//作为server接受一个client
static int net_lwip_socket_accept(int socket_id, uint64_t tag,  luat_ip_addr_t *remote_ip, uint16_t *remote_port, void *user_data)
{
//	uint8_t temp[16];
//	int result = net_lwip_check_socket(user_data, socket_id, tag);
//	if (result) return result;
	platform_send_event(prvlwip.task_handle, EV_LWIP_SOCKET_ACCPET, socket_id, 0, user_data);
//	remote_ip->is_ipv6 = 0;
//	remote_ip->ipv4 = BytesGetLe32(temp);
//	*remote_port = BytesGetBe16(temp + 4);
//	NET_DBG("client %d.%d.%d.%d, %u", temp[0], temp[1], temp[2], temp[3], *remote_port);
	return 0;
}
//主动断开一个tcp连接，需要走完整个tcp流程，用户需要接收到close ok回调才能确认彻底断开
static int net_lwip_socket_disconnect(int socket_id, uint64_t tag,  void *user_data)
{
	int result = net_lwip_check_socket(user_data, socket_id, tag);
	if (result) return result;
	platform_send_event(prvlwip.task_handle, EV_LWIP_SOCKET_CLOSE, socket_id, 1, user_data);
	return 0;
}

static int net_lwip_socket_force_close(int socket_id, void *user_data)
{
	prvlwip.socket[socket_id].state = 1;
	platform_send_event(prvlwip.task_handle, EV_LWIP_SOCKET_CLOSE, socket_id, 0, user_data);
	return 0;
}

static int net_lwip_socket_close(int socket_id, uint64_t tag,  void *user_data)
{
	int result = net_lwip_check_socket(user_data, socket_id, tag);
	if (result) return result;
	net_lwip_socket_force_close(socket_id, user_data);
	return 0;

}

static uint32_t net_lwip_socket_read_data(int socket_id, uint8_t *buf, uint32_t *read_len, uint32_t len, socket_data_t *p)
{
	uint32_t dummy_len;
	dummy_len = ((p->len - p->read_pos) > (len - *read_len))?(len - *read_len):(p->len - p->read_pos);
	memcpy(buf, p->data + p->read_pos, dummy_len);
	p->read_pos += dummy_len;
	if (p->read_pos >= p->len)
	{
		if (prvlwip.socket[socket_id].is_tcp)
		{
			platform_send_event(prvlwip.task_handle, EV_LWIP_SOCKET_RX_DONE, socket_id, p->len, 0);
		}
		llist_del(&p->node);
		free(p->data);
		free(p);
	}
	*read_len += dummy_len;
	return dummy_len;
}

static int net_lwip_socket_receive(int socket_id, uint64_t tag,  uint8_t *buf, uint32_t len, int flags, luat_ip_addr_t *remote_ip, uint16_t *remote_port, void *user_data)
{
	int result = net_lwip_check_socket(user_data, socket_id, tag);
	if (result) return result;

	uint32_t read_len = 0;
	if (buf)
	{
		SOCKET_LOCK(socket_id);
		socket_data_t *p = (socket_data_t *)llist_traversal(&prvlwip.socket[socket_id].rx_head, net_lwip_next_data_cache, &prvlwip.socket[socket_id]);

		if (prvlwip.socket[socket_id].is_tcp)
		{
			while((read_len < len) && p)
			{
				prvlwip.socket[socket_id].rx_wait_size -= net_lwip_socket_read_data(socket_id, buf + read_len, &read_len, len, p);
				p = (socket_data_t *)llist_traversal(&prvlwip.socket[socket_id].rx_head, net_lwip_next_data_cache, &prvlwip.socket[socket_id]);
			}
		}
		else
		{
			prvlwip.socket[socket_id].rx_wait_size -= net_lwip_socket_read_data(socket_id, buf + read_len, &read_len, len, p);
		}
		if (llist_empty(&prvlwip.socket[socket_id].rx_head))
		{
			prvlwip.socket[socket_id].rx_wait_size = 0;
		}
		SOCKET_UNLOCK(socket_id);
	}
	else
	{
		read_len = prvlwip.socket[socket_id].rx_wait_size;
	}

	return read_len;
}
static int net_lwip_socket_send(int socket_id, uint64_t tag, const uint8_t *buf, uint32_t len, int flags, luat_ip_addr_t *remote_ip, uint16_t remote_port, void *user_data)
{
	int result = net_lwip_check_socket(user_data, socket_id, tag);
	if (result) return result;
	SOCKET_LOCK(socket_id);
	uint32_t save_len = 0;
	uint32_t dummy_len = 0;
	socket_data_t *p;
	if (prvlwip.socket[socket_id].is_tcp)
	{
		while(save_len < len)
		{
			dummy_len = ((len - save_len) > SOCKET_BUF_LEN)?SOCKET_BUF_LEN:(len - save_len);


			p = net_lwip_create_data_node(socket_id, &buf[save_len], dummy_len, remote_ip, remote_port);

			if (p)
			{
				llist_add_tail(&p->node, &prvlwip.socket[socket_id].tx_head);
			}
			else
			{
				SOCKET_UNLOCK(socket_id);
				return -1;
			}
			save_len += dummy_len;
		}
	}
	else
	{
		p = net_lwip_create_data_node(socket_id, buf, len, remote_ip, remote_port);
		if (p)
		{
			llist_add_tail(&p->node, &prvlwip.socket[socket_id].tx_head);
		}
		else
		{
			SOCKET_UNLOCK(socket_id);
			return -1;
		}
	}

	SOCKET_UNLOCK(socket_id);
	platform_send_event(prvlwip.task_handle, EV_LWIP_SOCKET_TX, socket_id, 0, user_data);
	result = len;
	return result;
}

void net_lwip_socket_clean(int *vaild_socket_list, uint32_t num, void *user_data)
{
	if ((uint32_t)user_data >= NW_ADAPTER_INDEX_LWIP_NETIF_QTY) return;
	int socket_list[MAX_SOCK_NUM];
	memset(socket_list, 0, sizeof(socket_list));
	uint32_t i;
	for(i = 0; i < num + 1; i++)
	{
		if ( (vaild_socket_list[i] > 0) && (vaild_socket_list[i] < MAX_SOCK_NUM) )
		{
			socket_list[vaild_socket_list[i]] = 1;
		}
		NET_DBG("%d,%d",i,vaild_socket_list[i]);
	}
	for(i = 0; i < MAX_SOCK_NUM; i++)
	{
		NET_DBG("%d,%d",i,socket_list[i]);
		if ( !socket_list[i] )
		{
			net_lwip_socket_force_close(i, user_data);
		}
	}
}

static int net_lwip_getsockopt(int socket_id, uint64_t tag,  int level, int optname, void *optval, uint32_t *optlen, void *user_data)
{
	int result = net_lwip_check_socket(user_data, socket_id, tag);
	if (result) return result;
	switch (level)
	{

	    /* Level: SOL_SOCKET */
	case SOL_SOCKET:
		switch (optname)
		{

#if LWIP_TCP
		case SO_ACCEPTCONN:
			if (!prvlwip.socket[socket_id].is_tcp) {
				return -1;
			}
			if ((prvlwip.socket[socket_id].pcb.tcp != NULL) && (prvlwip.socket[socket_id].pcb.tcp->state == LISTEN)) {
				*(int *)optval = 1;
			} else {
				*(int *)optval = 0;
			}
			break;
#endif /* LWIP_TCP */

	        /* The option flags */
	    case SO_BROADCAST:
	    case SO_KEEPALIVE:
#if SO_REUSE
	    case SO_REUSEADDR:
#endif /* SO_REUSE */
			if ((optname == SO_BROADCAST) && prvlwip.socket[socket_id].is_tcp) {
				return -1;
			}

			optname = lwip_sockopt_to_ipopt(optname);
			*(int *)optval = ip_get_option(prvlwip.socket[socket_id].pcb.ip, optname);
			break;
	    case SO_TYPE:
	    	if (prvlwip.socket[socket_id].is_tcp)
	    	{
	    		*(int *)optval = SOCK_STREAM;
	    	}
	    	else
	    	{
	    		*(int *)optval = SOCK_DGRAM;
	    	}
	        break;

	    case SO_ERROR:
	    	return -1;
	        break;
#if LWIP_UDP
	    case SO_NO_CHECK:
	    	if (prvlwip.socket[socket_id].is_tcp) return -1;
#if LWIP_UDPLITE
	        if (udp_is_flag_set(prvlwip.socket[socket_id].pcb.udp, UDP_FLAGS_UDPLITE)) {
	            /* this flag is only available for UDP, not for UDP lite */
	            done_socket(sock);
	            return -1;
	        }
#endif /* LWIP_UDPLITE */
			*(int *)optval = udp_is_flag_set(prvlwip.socket[socket_id].pcb.udp, UDP_FLAGS_NOCHKSUM) ? 1 : 0;
			break;
	#endif /* LWIP_UDP*/
	    default:
			return -1;
			break;
		}  /* switch (optname) */
		break;

	    /* Level: IPPROTO_IP */
	case IPPROTO_IP:
		switch (optname) {
	        case IP_TTL:
				*(int *)optval = prvlwip.socket[socket_id].pcb.ip->ttl;
				break;
	        case IP_TOS:
				*(int *)optval = prvlwip.socket[socket_id].pcb.ip->tos;
				break;
#if LWIP_IPV4 && LWIP_MULTICAST_TX_OPTIONS && LWIP_UDP
	        case IP_MULTICAST_TTL:
	        	if (prvlwip.socket[socket_id].is_tcp) return -1;
				*(u8_t *)optval = udp_get_multicast_ttl(prvlwip.socket[socket_id].pcb.udp);

				break;
	        case IP_MULTICAST_IF:
	        	if (prvlwip.socket[socket_id].is_tcp) return -1;
	        	((struct in_addr *)optval)->s_addr = prvlwip.socket[socket_id].pcb.udp->mcast_ip4.addr;
	        	break;
	        case IP_MULTICAST_LOOP:
	        	if (prvlwip.socket[socket_id].is_tcp) return -1;
				if ((prvlwip.socket[socket_id].pcb.udp->flags & UDP_FLAGS_MULTICAST_LOOP) != 0) {
					*(u8_t *)optval = 1;
				} else {
					*(u8_t *)optval = 0;
				}
				break;
	#endif /* LWIP_IPV4 && LWIP_MULTICAST_TX_OPTIONS && LWIP_UDP */
	        default:
	        	return -1;
		}  /* switch (optname) */
		break;

#if LWIP_TCP
	    /* Level: IPPROTO_TCP */
	case IPPROTO_TCP:
	      /* Special case: all IPPROTO_TCP option take an int */
		if (!prvlwip.socket[socket_id].is_tcp) return -1;
		if (prvlwip.socket[socket_id].pcb.tcp->state == LISTEN) {
			return -1;
		}
	    switch (optname) {
			case TCP_NODELAY:
				*(int *)optval = tcp_nagle_disabled(prvlwip.socket[socket_id].pcb.tcp);
				break;
			case TCP_KEEPALIVE:
				*(int *)optval = (int)prvlwip.socket[socket_id].pcb.tcp->keep_idle;
				break;

#if LWIP_TCP_KEEPALIVE
			case TCP_KEEPIDLE:
				*(int *)optval = (int)(prvlwip.socket[socket_id].pcb.tcp->keep_idle / 1000);
				break;
			case TCP_KEEPINTVL:
				*(int *)optval = (int)(prvlwip.socket[socket_id].pcb.tcp->keep_intvl / 1000);

				break;
			case TCP_KEEPCNT:
				*(int *)optval = (int)prvlwip.socket[socket_id].pcb.tcp->keep_cnt;
				break;
#endif /* LWIP_TCP_KEEPALIVE */
			default:
				return -1;
				break;
		}  /* switch (optname) */
		break;
	#endif /* LWIP_TCP */

	#if LWIP_UDP && LWIP_UDPLITE
	    /* Level: IPPROTO_UDPLITE */
	case IPPROTO_UDPLITE:
	      /* Special case: all IPPROTO_UDPLITE option take an int */
	      LWIP_SOCKOPT_CHECK_OPTLEN_CONN_PCB(sock, *optlen, int);
	      /* If this is no UDP lite socket, ignore any options. */
	      if (!NETCONNTYPE_ISUDPLITE(netconn_type(sock->conn))) {
	        done_socket(sock);
	        return ENOPROTOOPT;
	      }
	      switch (optname) {
	        case UDPLITE_SEND_CSCOV:
	          *(int *)optval = prvlwip.socket[socket_id].pcb.udp->chksum_len_tx;
	          LWIP_DEBUGF(SOCKETS_DEBUG, ("lwip_getsockopt(%d, IPPROTO_UDPLITE, UDPLITE_SEND_CSCOV) = %d\n",
	                                      s, (*(int *)optval)) );
	          break;
	        case UDPLITE_RECV_CSCOV:
	          *(int *)optval = prvlwip.socket[socket_id].pcb.udp->chksum_len_rx;
	          LWIP_DEBUGF(SOCKETS_DEBUG, ("lwip_getsockopt(%d, IPPROTO_UDPLITE, UDPLITE_RECV_CSCOV) = %d\n",
	                                      s, (*(int *)optval)) );
	          break;
	        default:
	          LWIP_DEBUGF(SOCKETS_DEBUG, ("lwip_getsockopt(%d, IPPROTO_UDPLITE, UNIMPL: optname=0x%x, ..)\n",
	                                      s, optname));
	          err = ENOPROTOOPT;
	          break;
	      }  /* switch (optname) */
	      break;
	#endif /* LWIP_UDP */
	    /* Level: IPPROTO_RAW */
	case IPPROTO_RAW:
		return -1;
	default:
		return -1;
		break;
	} /* switch (level) */

	return 0;
}
static int net_lwip_setsockopt(int socket_id, uint64_t tag,  int level, int optname, const void *optval, uint32_t optlen, void *user_data)
{
	int result = net_lwip_check_socket(user_data, socket_id, tag);
	if (result) return result;
	switch (level)
	{

	    /* Level: SOL_SOCKET */
	case SOL_SOCKET:
		switch (optname)
		{

		/* SO_ACCEPTCONN is get-only */

		/* The option flags */
		case SO_BROADCAST:
		case SO_KEEPALIVE:
#if SO_REUSE
		case SO_REUSEADDR:
#endif /* SO_REUSE */
			if ((optname == SO_BROADCAST) &&
			  (prvlwip.socket[socket_id].is_tcp)) {
				return -1;
			}

			optname = lwip_sockopt_to_ipopt(optname);

			if (*(const int *)optval) {
				ip_set_option(prvlwip.socket[socket_id].pcb.ip, optname);
			} else {
				ip_reset_option(prvlwip.socket[socket_id].pcb.ip, optname);
			}
			break;
#if LWIP_UDP
		case SO_NO_CHECK:
			if (prvlwip.socket[socket_id].is_tcp) return -1;

			if (*(const int *)optval) {
				udp_set_flags(prvlwip.socket[socket_id].pcb.udp, UDP_FLAGS_NOCHKSUM);
			} else {
				udp_clear_flags(prvlwip.socket[socket_id].pcb.udp, UDP_FLAGS_NOCHKSUM);
			}
	          break;
	#endif /* LWIP_UDP */

		default:
			return -1;
	    }  /* switch (optname) */
	    break;

	    /* Level: IPPROTO_IP */
	case IPPROTO_IP:
	    switch (optname)
	    {
	    case IP_TTL:
			prvlwip.socket[socket_id].pcb.ip->ttl = (u8_t)(*(const int *)optval);

			break;
	    case IP_TOS:
	        prvlwip.socket[socket_id].pcb.ip->tos = (u8_t)(*(const int *)optval);
	        break;
#if LWIP_NETBUF_RECVINFO
	    case IP_PKTINFO:

			if (*(const int *)optval) {
				prvlwip.socket[socket_id].flags |= NETCONN_FLAG_PKTINFO;
			} else {
				prvlwip.socket[socket_id].flags &= ~NETCONN_FLAG_PKTINFO;
			}
			break;
#endif /* LWIP_NETBUF_RECVINFO */
#if LWIP_IPV4 && LWIP_MULTICAST_TX_OPTIONS && LWIP_UDP
		case IP_MULTICAST_TTL:
			if (prvlwip.socket[socket_id].is_tcp) return -1;
			udp_set_multicast_ttl(prvlwip.socket[socket_id].pcb.udp, (u8_t)(*(const u8_t *)optval));
			break;
		case IP_MULTICAST_IF:
			if (prvlwip.socket[socket_id].is_tcp) return -1;
			prvlwip.socket[socket_id].pcb.udp->mcast_ip4.addr = ((struct in_addr *)optval)->s_addr;
			break;
		case IP_MULTICAST_LOOP:
			if (*(const u8_t *)optval) {
				udp_set_flags(prvlwip.socket[socket_id].pcb.udp, UDP_FLAGS_MULTICAST_LOOP);
			} else {
				udp_clear_flags(prvlwip.socket[socket_id].pcb.udp, UDP_FLAGS_MULTICAST_LOOP);
			}
			break;
#endif /* LWIP_IPV4 && LWIP_MULTICAST_TX_OPTIONS && LWIP_UDP */
		default:
			return -1;
			break;
		}  /* switch (optname) */
		break;

#if LWIP_TCP
	    /* Level: IPPROTO_TCP */
	case IPPROTO_TCP:
	      /* Special case: all IPPROTO_TCP option take an int */
		if (!prvlwip.socket[socket_id].is_tcp) return -1;
	    if (prvlwip.socket[socket_id].pcb.tcp->state == LISTEN) {
	    	return -1;
	    }
		switch (optname)
		{
		case TCP_NODELAY:
			if (*(const int *)optval) {
				tcp_nagle_disable(prvlwip.socket[socket_id].pcb.tcp);
			} else {
				tcp_nagle_enable(prvlwip.socket[socket_id].pcb.tcp);
			}
			break;
		case TCP_KEEPALIVE:
			prvlwip.socket[socket_id].pcb.tcp->keep_idle = (u32_t)(*(const int *)optval);

			break;

#if LWIP_TCP_KEEPALIVE
		case TCP_KEEPIDLE:
			prvlwip.socket[socket_id].pcb.tcp->keep_idle = 1000 * (u32_t)(*(const int *)optval);
			break;
		case TCP_KEEPINTVL:
			prvlwip.socket[socket_id].pcb.tcp->keep_intvl = 1000 * (u32_t)(*(const int *)optval);

			break;
		case TCP_KEEPCNT:
			prvlwip.socket[socket_id].pcb.tcp->keep_cnt = (u32_t)(*(const int *)optval);

			break;
#endif /* LWIP_TCP_KEEPALIVE */
		default:
			return -1;
			break;
		}  /* switch (optname) */
		break;
#endif /* LWIP_TCP*/

#if LWIP_IPV6
	    /* Level: IPPROTO_IPV6 */
	case IPPROTO_IPV6:
		return -1;
		break;
#endif /* LWIP_IPV6 */


	    /* Level: IPPROTO_RAW */
	case IPPROTO_RAW:
	    return -1;
	    break;
	default:
		return -1;
	}   /* switch (level) */
	return 0;
}
static int net_lwip_get_local_ip_info(luat_ip_addr_t *ip, luat_ip_addr_t *submask, luat_ip_addr_t *gateway, void *user_data)
{
	uint8_t index = (uint32_t)user_data;
	if (index >= NW_ADAPTER_INDEX_LWIP_NETIF_QTY) return -1;
	if (!prvlwip.lwip_netif[index]) return -1;
	*ip = prvlwip.lwip_netif[index]->ip_addr;
	*submask = prvlwip.lwip_netif[index]->netmask;
	*gateway = prvlwip.lwip_netif[index]->gw;
	return 0;
}

static int net_lwip_user_cmd(int socket_id, uint64_t tag, uint32_t cmd, uint32_t value, void *user_data)
{
	return 0;
}

static int net_lwip_dns(const char *domain_name, uint32_t len, void *param, void *user_data)
{
	if ((uint32_t)user_data >= NW_ADAPTER_INDEX_LWIP_NETIF_QTY) return -1;
	char *prv_domain_name = (char *)OS_Zalloc(len + 1);
	memcpy(prv_domain_name, domain_name, len);
	platform_send_event(prvlwip.task_handle, EV_LWIP_SOCKET_DNS, prv_domain_name, param, user_data);
	return 0;
}

static int net_lwip_set_dns_server(uint8_t server_index, luat_ip_addr_t *ip, void *user_data)
{
	if ((uint32_t)user_data >= NW_ADAPTER_INDEX_LWIP_NETIF_QTY) return -1;
	if (server_index >= MAX_DNS_SERVER) return -1;
	prvlwip.dns_client.dns_server[server_index] = *ip;
	prvlwip.dns_client.is_static_dns[server_index] = 1;
	return 0;
}

static int net_lwip_set_mac(uint8_t *mac, void *user_data)
{
	uint8_t index = (uint32_t)user_data;
	if (index >= NW_ADAPTER_INDEX_LWIP_NETIF_QTY) return -1;
	if (!prvlwip.lwip_netif[index]) return -1;
	memcpy(prvlwip.lwip_netif[index]->hwaddr, mac, 6);
	return -1;
}
int net_lwip_set_static_ip(luat_ip_addr_t *ip, luat_ip_addr_t *submask, luat_ip_addr_t *gateway, luat_ip_addr_t *ipv6, void *user_data)
{
	uint8_t index = (uint32_t)user_data;
	if (index >= NW_ADAPTER_INDEX_LWIP_NETIF_QTY) return -1;
	if (!prvlwip.lwip_netif[index]) return -1;
	if (index == NW_ADAPTER_INDEX_LWIP_GPRS) return -1;
	luat_ip_addr_t *p_ip = zalloc(sizeof(luat_ip_addr_t) * 5);
	p_ip[0] = ip?(*ip):ip_addr_any_type;
	p_ip[1] = submask?(*submask):ip_addr_any_type;
	p_ip[2] = gateway?(*gateway):ip_addr_any_type;
	p_ip[3] = ipv6?(*ipv6):ip_addr_any_type;
	platform_send_event(prvlwip.task_handle, EV_LWIP_NETIF_SET_IP, p_ip, ipv6, user_data);
	return 0;
}

static int32_t net_lwip_dummy_callback(void *pData, void *pParam)
{
	return 0;
}

static void net_lwip_socket_set_callback(CBFuncEx_t cb_fun, void *param, void *user_data)
{
	uint8_t index = (uint32_t)user_data;
	if (index >= NW_ADAPTER_INDEX_LWIP_NETIF_QTY) return;
	prvlwip.socket_cb[index] = cb_fun?cb_fun:net_lwip_dummy_callback;
	prvlwip.user_data[index] = param;
}

static network_adapter_info prv_net_lwip_adapter =
{
		.check_ready = net_lwip_check_ready,
		.create_soceket = net_lwip_create_soceket,
		.socket_connect = net_lwip_socket_connect,
		.socket_listen = net_lwip_socket_listen,
		.socket_accept = net_lwip_socket_accept,
		.socket_disconnect = net_lwip_socket_disconnect,
		.socket_close = net_lwip_socket_close,
		.socket_force_close = net_lwip_socket_force_close,
		.socket_receive = net_lwip_socket_receive,
		.socket_send = net_lwip_socket_send,
		.socket_check = net_lwip_socket_check,
		.socket_clean = net_lwip_socket_clean,
		.getsockopt = net_lwip_getsockopt,
		.setsockopt = net_lwip_setsockopt,
		.user_cmd = net_lwip_user_cmd,
		.dns = net_lwip_dns,
		.set_dns_server = net_lwip_set_dns_server,
		.set_mac = net_lwip_set_mac,
		.set_static_ip = net_lwip_set_static_ip,
		.get_local_ip_info = net_lwip_get_local_ip_info,
		.socket_set_callback = net_lwip_socket_set_callback,
		.name = "lwip",
		.max_socket_num = MAX_SOCK_NUM,
		.no_accept = 0,
		.is_posix = 1,
};


void net_lwip_register_adapter(uint8_t adapter_index)
{
	network_register_adapter(adapter_index, &prv_net_lwip_adapter, adapter_index);
}

int net_lwip_check_all_ack(int socket_id)
{
	if (!llist_empty(&prvlwip.socket[socket_id].wait_ack_head))
	{
		NET_DBG("socekt %d not all ack", socket_id);
		return -1;
	}
	if (!llist_empty(&prvlwip.socket[socket_id].tx_head))
	{
		NET_DBG("socekt %d not all send", socket_id);
		return -1;
	}
	if (prvlwip.socket[socket_id].pcb.tcp->snd_buf != TCP_SND_BUF)
	{
		NET_DBG("socket_id %d send buf %ubytes, need %u", prvlwip.socket[socket_id].pcb.tcp->snd_buf, TCP_SND_BUF);
	}
	return 0;
}

void net_lwip_set_netif(uint8_t adapter_index, struct netif *netif, void *init, uint8_t is_default)
{
	if (!netif)
	{
		netif = zalloc(sizeof(struct netif));
	}

	prvlwip.lwip_netif[adapter_index] = netif;
	netif_add(netif, NULL, NULL, NULL, NULL, init, netif_input);
	netif->dhcp_done_callback = net_lwip_dhcp_done_cb;
	netif->dhcp_done_arg = (void *)adapter_index;
//	switch(adapter_index)
//	{
//	case NW_ADAPTER_INDEX_LWIP_WIFI_STA:
//	case NW_ADAPTER_INDEX_LWIP_WIFI_AP:
//	case NW_ADAPTER_INDEX_LWIP_ETH:
//		prvlwip.lwip_netif[adapter_index]->flags |= (NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET|NETIF_FLAG_BROADCAST);
//		break;
//	}
	if (is_default)
	{
		prvlwip.dns_udp->netif_idx = netif_get_index(netif);
		netif_set_default(netif);
	}
}

struct netif * net_lwip_get_netif(uint8_t adapter_index)
{
	return prvlwip.lwip_netif[adapter_index];
}

void net_lwip_input_packets(struct netif *netif, struct pbuf *p)
{
	platform_send_event(prvlwip.task_handle, EV_LWIP_NETIF_INPUT, p, 0, netif);
}

void net_lwip_set_link_state(uint8_t adapter_index, uint8_t onoff)
{
	platform_send_event(prvlwip.task_handle, EV_LWIP_NETIF_LINK_STATE, onoff, 0, adapter_index);
}


void net_lwip_set_ipv6_by_mac(uint8_t adapter_index)
{
	platform_send_event(prvlwip.task_handle, EV_LWIP_NETIF_IPV6_BY_MAC, 0, 0, adapter_index);
}
