/*
 * Copyright 2013-16 Board of Trustees of Stanford University
 * Copyright 2013-16 Ecole Polytechnique Federale Lausanne (EPFL)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*-
 * Copyright (c) 1982, 1986, 1993
 *	The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)ip.h	8.2 (Berkeley) 6/1/94
 * $FreeBSD$
 */

#pragma once

#include <linux/types.h>

#include <kns/cpu.h>

/*
 * Definitions for internet protocol version 4.
 *
 * Per RFC 791, September 1981.
 */
#define	IPVERSION	4

struct ip_addr {
	uint32_t addr;
} __packed;

#define MAKE_IP_ADDR(a, b, c, d)			\
	(((uint32_t) a << 24) | ((uint32_t) b << 16) |	\
	 ((uint32_t) c << 8) | (uint32_t) d)

#define IP_ADDR_STR_LEN	16

extern void ip_addr_to_str(struct ip_addr *addr, char *str);

/*
 * Structure of an internet header, naked of options.
 */
struct ip_hdr {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	uint8_t	header_len: 4,		/* header length */
		version: 4;		/* version */
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
	uint8_t	version: 4,		/* version */
		header_len: 4;		/* header length */
#endif
	uint8_t tos;			/* type of service */
	uint16_t len;			/* total length */
	uint16_t id;			/* identification */
	uint16_t off;			/* fragment offset field */
#define	IP_RF 0x8000			/* reserved fragment flag */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
	uint8_t ttl;			/* time to live */
	uint8_t proto;			/* protocol */
	uint16_t chksum;		/* checksum */
	struct	ip_addr src_addr;	/* source address */
	struct  ip_addr dst_addr;	/* dest address */
} __packed __aligned(4);

#define	IP_MAXPACKET	65535		/* maximum packet size */

/*
 * Definitions for IP type of service (ip_tos).
 */
#define	LWIP_IPTOS_LOWDELAY		0x10
#define	LWIP_IPTOS_THROUGHPUT	0x08
#define	LWIP_IPTOS_RELIABILITY	0x04
#define	LWIP_IPTOS_MINCOST		0x02

/*
 * Definitions for IP precedence (also in ip_tos) (hopefully unused).
 */
#define	LWIP_IPTOS_PREC_NETCONTROL		0xe0
#define	LWIP_IPTOS_PREC_INTERNETCONTROL	0xc0
#define	LWIP_IPTOS_PREC_CRITIC_ECP		0xa0
#define	LWIP_IPTOS_PREC_FLASHOVERRIDE	0x80
#define	LWIP_IPTOS_PREC_FLASH		0x60
#define	LWIP_IPTOS_PREC_IMMEDIATE		0x40
#define	LWIP_IPTOS_PREC_PRIORITY		0x20
#define	LWIP_IPTOS_PREC_ROUTINE		0x00

/*
 * Definitions for DiffServ Codepoints as per RFC2474
 */
#define	LWIP_IPTOS_DSCP_CS0		0x00
#define	LWIP_IPTOS_DSCP_CS1		0x20
#define	LWIP_IPTOS_DSCP_AF11		0x28
#define	LWIP_IPTOS_DSCP_AF12		0x30
#define	LWIP_IPTOS_DSCP_AF13		0x38
#define	LWIP_IPTOS_DSCP_CS2		0x40
#define	LWIP_IPTOS_DSCP_AF21		0x48
#define	LWIP_IPTOS_DSCP_AF22		0x50
#define	LWIP_IPTOS_DSCP_AF23		0x58
#define	LWIP_IPTOS_DSCP_CS3		0x60
#define	LWIP_IPTOS_DSCP_AF31		0x68
#define	LWIP_IPTOS_DSCP_AF32		0x70
#define	LWIP_IPTOS_DSCP_AF33		0x78
#define	LWIP_IPTOS_DSCP_CS4		0x80
#define	LWIP_IPTOS_DSCP_AF41		0x88
#define	LWIP_IPTOS_DSCP_AF42		0x90
#define	LWIP_IPTOS_DSCP_AF43		0x98
#define	LWIP_IPTOS_DSCP_CS5		0xa0
#define	LWIP_IPTOS_DSCP_EF		0xb8
#define	LWIP_IPTOS_DSCP_CS6		0xc0
#define	LWIP_IPTOS_DSCP_CS7		0xe0

/*
 * ECN (Explicit Congestion Notification) codepoints in RFC3168 mapped to the
 * lower 2 bits of the TOS field.
 */
#define	LWIP_IPTOS_ECN_NOTECT	0x00	/* not-ECT */
#define	LWIP_IPTOS_ECN_ECT1		0x01	/* ECN-capable transport (1) */
#define	LWIP_IPTOS_ECN_ECT0		0x02	/* ECN-capable transport (0) */
#define	LWIP_IPTOS_ECN_CE		0x03	/* congestion experienced */
#define	LWIP_IPTOS_ECN_MASK		0x03	/* ECN field mask */

/*
 * Definitions for options.
 */
#define	LWIP_IPOPT_COPIED(o)		((o)&0x80)
#define	LWIP_IPOPT_CLASS(o)		((o)&0x60)
#define	LWIP_IPOPT_NUMBER(o)		((o)&0x1f)

#define	LWIP_IPOPT_CONTROL		0x00
#define	LWIP_IPOPT_RESERVED1		0x20
#define	LWIP_IPOPT_DEBMEAS		0x40
#define	LWIP_IPOPT_RESERVED2		0x60

#define	LWIP_IPOPT_EOL		0		/* end of option list */
#define	LWIP_IPOPT_NOP		1		/* no operation */

#define	LWIP_IPOPT_RR		7		/* record packet route */
#define	LWIP_IPOPT_TS		68		/* timestamp */
#define	LWIP_IPOPT_SECURITY		130		/* provide s,c,h,tcc */
#define	LWIP_IPOPT_LSRR		131		/* loose source route */
#define	LWIP_IPOPT_ESO		133		/* extended security */
#define	LWIP_IPOPT_CIPSO		134		/* commerical security */
#define	LWIP_IPOPT_SATID		136		/* satnet id */
#define	LWIP_IPOPT_SSRR		137		/* strict source route */
#define	LWIP_IPOPT_RA		148		/* router alert */

/*
 * Offsets to fields in options other than EOL and NOP.
 */
#define	LWIP_IPOPT_OPTVAL		0		/* option ID */
#define	LWIP_IPOPT_OLEN		1		/* option length */
#define	LWIP_IPOPT_OFFSET		2		/* offset within option */
#define	LWIP_IPOPT_MINOFF		4		/* min value of above */

/*
 * Time stamp option structure.
 */
struct	ip_timestamp {
	uint8_t code;			/* LWIP_IPOPT_TS */
	uint8_t len;			/* size of structure (variable) */
	uint8_t ptr;			/* index of current entry */
#if __BYTE_ORDER == __LITTLE_ENDIAN
	uint8_t flags: 4,		/* flags, see below */
		overflow: 4;		/* overflow counter */
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
	uint8_t overflow: 4,		/* overflow counter */
		flags: 4;		/* flags, see below */
#endif
	union  {
		uint32_t time[1];	/* network format */
		struct {
			struct ip_addr addr;
			uint32_t ipt_time;	/* network format */
		} ta[1];
	} u;
};

/* Flag bits for ipt_flg. */
#define	LWIP_IPOPT_TS_TSONLY		0		/* timestamps only */
#define	LWIP_IPOPT_TS_TSANDADDR	1		/* timestamps and addresses */
#define	LWIP_IPOPT_TS_PRESPEC	3		/* specified modules only */

/* Bits for security (not byte swapped). */
#define	LWIP_IPOPT_SECUR_UNCLASS	0x0000
#define	LWIP_IPOPT_SECUR_CONFID	0xf135
#define	LWIP_IPOPT_SECUR_EFTO	0x789a
#define	LWIP_IPOPT_SECUR_MMMM	0xbc4d
#define	LWIP_IPOPT_SECUR_RESTR	0xaf13
#define	LWIP_IPOPT_SECUR_SECRET	0xd788
#define	LWIP_IPOPT_SECUR_TOPSECRET	0x6bc5

/*
 * Internet implementation parameters.
 */
#define	LWIP_MAXTTL		255		/* maximum time to live (seconds) */
#define	LWIP_IPDEFTTL	64		/* default ttl, from RFC 1340 */
#define	LWIP_IPFRAGTTL	60		/* time to live for frags, slowhz */
#define	LWIP_IPTTLDEC	1		/* subtracted when forwarding */
#define	LWIP_IP_MSS		576		/* default maximum segment size */

/*
 * This is the real IPv4 pseudo header, used for computing the TCP and UDP
 * checksums. For the Internet checksum, struct ipovly can be used instead.
 * For stronger checksums, the real thing must be used.
 */
struct ip_pseudo {
	struct	ip_addr	src;		/* source internet address */
	struct	ip_addr	dst;		/* destination internet address */
	uint8_t		pad;		/* pad, must be zero */
	uint8_t		proto;		/* protocol */
	uint16_t	len;		/* protocol length */
};

/* Protocols common to RFC 1700, POSIX, and X/Open. */
#define	LWIP_IPPROTO_IP		0		/* dummy for IP */
#define	LWIP_IPPROTO_ICMP		1		/* control message protocol */
#define	LWIP_IPPROTO_TCP		6		/* tcp */
#define	LWIP_IPPROTO_UDP		17		/* user datagram protocol */

/* Protocols (RFC 1700) */
#define	LWIP_IPPROTO_HOPOPTS		0		/* IP6 hop-by-hop options */
#define	LWIP_IPPROTO_IGMP		2		/* group mgmt protocol */
#define	LWIP_IPPROTO_GGP		3		/* gateway^2 (deprecated) */
#define	LWIP_IPPROTO_IPV4		4		/* IPv4 encapsulation */
#define	LWIP_IPPROTO_IPIP		LWIP_IPPROTO_IPV4	/* for compatibility */
#define	LWIP_IPPROTO_ST		7		/* Stream protocol II */
#define	LWIP_IPPROTO_EGP		8		/* exterior gateway protocol */
#define	LWIP_IPPROTO_PIGP		9		/* private interior gateway */
#define	LWIP_IPPROTO_RCCMON		10		/* BBN RCC Monitoring */
#define	LWIP_IPPROTO_NVPII		11		/* network voice protocol*/
#define	LWIP_IPPROTO_PUP		12		/* pup */
#define	LWIP_IPPROTO_ARGUS		13		/* Argus */
#define	LWIP_IPPROTO_EMCON		14		/* EMCON */
#define	LWIP_IPPROTO_XNET		15		/* Cross Net Debugger */
#define	LWIP_IPPROTO_CHAOS		16		/* Chaos*/
#define	LWIP_IPPROTO_MUX		18		/* Multiplexing */
#define	LWIP_IPPROTO_MEAS		19		/* DCN Measurement Subsystems */
#define	LWIP_IPPROTO_HMP		20		/* Host Monitoring */
#define	LWIP_IPPROTO_PRM		21		/* Packet Radio Measurement */
#define	LWIP_IPPROTO_IDP		22		/* xns idp */
#define	LWIP_IPPROTO_TRUNK1		23		/* Trunk-1 */
#define	LWIP_IPPROTO_TRUNK2		24		/* Trunk-2 */
#define	LWIP_IPPROTO_LEAF1		25		/* Leaf-1 */
#define	LWIP_IPPROTO_LEAF2		26		/* Leaf-2 */
#define	LWIP_IPPROTO_RDP		27		/* Reliable Data */
#define	LWIP_IPPROTO_IRTP		28		/* Reliable Transaction */
#define	LWIP_IPPROTO_TP		29		/* tp-4 w/ class negotiation */
#define	LWIP_IPPROTO_BLT		30		/* Bulk Data Transfer */
#define	LWIP_IPPROTO_NSP		31		/* Network Services */
#define	LWIP_IPPROTO_INP		32		/* Merit Internodal */
#define	LWIP_IPPROTO_SEP		33		/* Sequential Exchange */
#define	LWIP_IPPROTO_3PC		34		/* Third Party Connect */
#define	LWIP_IPPROTO_IDPR		35		/* InterDomain Policy Routing */
#define	LWIP_IPPROTO_XTP		36		/* XTP */
#define	LWIP_IPPROTO_DDP		37		/* Datagram Delivery */
#define	LWIP_IPPROTO_CMTP		38		/* Control Message Transport */
#define	LWIP_IPPROTO_TPXX		39		/* TP++ Transport */
#define	LWIP_IPPROTO_IL		40		/* IL transport protocol */
#define	LWIP_IPPROTO_IPV6		41		/* IP6 header */
#define	LWIP_IPPROTO_SDRP		42		/* Source Demand Routing */
#define	LWIP_IPPROTO_ROUTING		43		/* IP6 routing header */
#define	LWIP_IPPROTO_FRAGMENT	44		/* IP6 fragmentation header */
#define	LWIP_IPPROTO_IDRP		45		/* InterDomain Routing*/
#define	LWIP_IPPROTO_RSVP		46		/* resource reservation */
#define	LWIP_IPPROTO_GRE		47		/* General Routing Encap. */
#define	LWIP_IPPROTO_MHRP		48		/* Mobile Host Routing */
#define	LWIP_IPPROTO_BHA		49		/* BHA */
#define	LWIP_IPPROTO_ESP		50		/* IP6 Encap Sec. Payload */
#define	LWIP_IPPROTO_AH		51		/* IP6 Auth Header */
#define	LWIP_IPPROTO_INLSP		52		/* Integ. Net Layer Security */
#define	LWIP_IPPROTO_SWIPE		53		/* IP with encryption */
#define	LWIP_IPPROTO_NHRP		54		/* Next Hop Resolution */
#define	LWIP_IPPROTO_MOBILE		55		/* IP Mobility */
#define	LWIP_IPPROTO_TLSP		56		/* Transport Layer Security */
#define	LWIP_IPPROTO_SKIP		57		/* SKIP */
#define	LWIP_IPPROTO_ICMPV6		58		/* ICMP6 */
#define	LWIP_IPPROTO_NONE		59		/* IP6 no next header */
#define	LWIP_IPPROTO_DSTOPTS		60		/* IP6 destination option */
#define	LWIP_IPPROTO_AHIP		61		/* any host internal protocol */
#define	LWIP_IPPROTO_CFTP		62		/* CFTP */
#define	LWIP_IPPROTO_HELLO		63		/* "hello" routing protocol */
#define	LWIP_IPPROTO_SATEXPAK	64		/* SATNET/Backroom EXPAK */
#define	LWIP_IPPROTO_KRYPTOLAN	65		/* Kryptolan */
#define	LWIP_IPPROTO_RVD		66		/* Remote Virtual Disk */
#define	LWIP_IPPROTO_IPPC		67		/* Pluribus Packet Core */
#define	LWIP_IPPROTO_ADFS		68		/* Any distributed FS */
#define	LWIP_IPPROTO_SATMON		69		/* Satnet Monitoring */
#define	LWIP_IPPROTO_VISA		70		/* VISA Protocol */
#define	LWIP_IPPROTO_IPCV		71		/* Packet Core Utility */
#define	LWIP_IPPROTO_CPNX		72		/* Comp. Prot. Net. Executive */
#define	LWIP_IPPROTO_CPHB		73		/* Comp. Prot. HeartBeat */
#define	LWIP_IPPROTO_WSN		74		/* Wang Span Network */
#define	LWIP_IPPROTO_PVP		75		/* Packet Video Protocol */
#define	LWIP_IPPROTO_BRSATMON	76		/* BackRoom SATNET Monitoring */
#define	LWIP_IPPROTO_ND		77		/* Sun net disk proto (temp.) */
#define	LWIP_IPPROTO_WBMON		78		/* WIDEBAND Monitoring */
#define	LWIP_IPPROTO_WBEXPAK		79		/* WIDEBAND EXPAK */
#define	LWIP_IPPROTO_EON		80		/* ISO cnlp */
#define	LWIP_IPPROTO_VMTP		81		/* VMTP */
#define	LWIP_IPPROTO_SVMTP		82		/* Secure VMTP */
#define	LWIP_IPPROTO_VINES		83		/* Banyon VINES */
#define	LWIP_IPPROTO_TTP		84		/* TTP */
#define	LWIP_IPPROTO_IGP		85		/* NSFNET-IGP */
#define	LWIP_IPPROTO_DGP		86		/* dissimilar gateway prot. */
#define	LWIP_IPPROTO_TCF		87		/* TCF */
#define	LWIP_IPPROTO_IGRP		88		/* Cisco/GXS IGRP */
#define	LWIP_IPPROTO_OSPFIGP		89		/* OSPFIGP */
#define	LWIP_IPPROTO_SRPC		90		/* Strite RPC protocol */
#define	LWIP_IPPROTO_LARP		91		/* Locus Address Resoloution */
#define	LWIP_IPPROTO_MTP		92		/* Multicast Transport */
#define	LWIP_IPPROTO_AX25		93		/* AX.25 Frames */
#define	LWIP_IPPROTO_IPEIP		94		/* IP encapsulated in IP */
#define	LWIP_IPPROTO_MICP		95		/* Mobile Int.ing control */
#define	LWIP_IPPROTO_SCCSP		96		/* Semaphore Comm. security */
#define	LWIP_IPPROTO_ETHERIP		97		/* Ethernet IP encapsulation */
#define	LWIP_IPPROTO_ENCAP		98		/* encapsulation header */
#define	LWIP_IPPROTO_APES		99		/* any private encr. scheme */
#define	LWIP_IPPROTO_GMTP		100		/* GMTP*/
#define	LWIP_IPPROTO_IPCOMP		108		/* payload compression (IPComp) */
#define	LWIP_IPPROTO_SCTP		132		/* SCTP */
#define	LWIP_IPPROTO_MH		135		/* IPv6 Mobility Header */
#define	LWIP_IPPROTO_HIP		139		/* IP6 Host Identity Protocol */
#define	LWIP_IPPROTO_SHIM6		140		/* IP6 Shim6 Protocol */
/* 101-254: Partly Unassigned */
#define	LWIP_IPPROTO_PIM		103		/* Protocol Independent Mcast */
#define	LWIP_IPPROTO_CARP		112		/* CARP */
#define	LWIP_IPPROTO_PGM		113		/* PGM */
#define	LWIP_IPPROTO_MPLS		137		/* MPLS-in-IP */
#define	LWIP_IPPROTO_PFSYNC		240		/* PFSYNC */
#define	LWIP_IPPROTO_RESERVED_253	253		/* Reserved */
#define	LWIP_IPPROTO_RESERVED_254	254		/* Reserved */
/* 255: Reserved */
/* BSD Private, local use, namespace incursion, no longer used */
#define	LWIP_IPPROTO_OLD_DIVERT	254		/* OLD divert pseudo-proto */
#define	LWIP_IPPROTO_RAW		255		/* raw IP packet */
#define	LWIP_IPPROTO_MAX		256