/*
* Copyright (c) 1999 - 2005 NetGroup, Politecnico di Torino (Italy)
* Copyright (c) 2005 - 2006 CACE Technologies, Davis (California)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* 3. Neither the name of the Politecnico di Torino, CACE Technologies
* nor the names of its contributors may be used to endorse or promote
* products derived from this software without specific prior written
* permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <pcap.h>
#include <Winsock2.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include "resource.h"

/* Ethernet addresses are 6 bytes */
#define ETHER_ADDR_LEN	6

/* ethernet headers are always exactly 14 bytes */
#define SIZE_ETHERNET 14

/* UDP headers are always exactly 8 bytes */
#define SIZE_UDP 14

/* Ethernet header */
struct ethernet_header {
	u_char ether_dhost[ETHER_ADDR_LEN]; /* Destination host address */
	u_char ether_shost[ETHER_ADDR_LEN]; /* Source host address */
	u_short ether_type; /* IP? ARP? RARP? etc */
};

///* 4 bytes IP address */
//typedef struct ip_address {
//	u_char byte1;
//	u_char byte2;
//	u_char byte3;
//	u_char byte4;
//}ip_address;

/* IPv4 header */
typedef struct ip_header {
	u_char	ver_ihl;		// Version (4 bits) + Internet header length (4 bits)
	u_char	tos;			// Type of service 
	u_short tlen;			// Total length 
	u_short identification; // Identification
	u_short flags_fo;		// Flags (3 bits) + Fragment offset (13 bits)
	u_char	ttl;			// Time to live
	u_char	proto;			// Protocol
	u_short crc;			// Header checksum
	struct in_addr ip_src, ip_dst; /* source and dest address */
	//ip_address	saddr;		// Source address
	//ip_address	daddr;		// Destination address
	u_int	op_pad;			// Option + Padding
}ip_header;

/* UDP header*/
typedef struct udp_header {
	u_short sport;			// Source port
	u_short dport;			// Destination port
	u_short len;			// Datagram length
	u_short crc;			// Checksum
}udp_header;

/* TCP header */
typedef u_int tcp_seq;

struct tcp_header {
	u_short th_sport;	/* source port */
	u_short th_dport;	/* destination port */
	tcp_seq th_seq;		/* sequence number */
	tcp_seq th_ack;		/* acknowledgement number */
	u_char th_offx2;	/* data offset, rsvd */
#define TH_OFF(th)	(((th)->th_offx2 & 0xf0) >> 4)
	u_char th_flags;
#define TH_FIN 0x01
#define TH_SYN 0x02
#define TH_RST 0x04
#define TH_PUSH 0x08
#define TH_ACK 0x10
#define TH_URG 0x20
#define TH_ECE 0x40
#define TH_CWR 0x80
#define TH_FLAGS (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
	u_short th_win;		/* window */
	u_short th_sum;		/* checksum */
	u_short th_urp;		/* urgent pointer */
};

BOOL LoadNpcapDlls()
{
	_TCHAR npcap_dir[512];
	UINT len;
	len = GetSystemDirectory(npcap_dir, 480);
	if (!len) {
		fprintf(stderr, "Error in GetSystemDirectory: %x", GetLastError());
		return FALSE;
	}
	_tcscat_s(npcap_dir, 512, _T("\\Npcap"));
	if (SetDllDirectory(npcap_dir) == 0) {
		fprintf(stderr, "Error in SetDllDirectory: %x", GetLastError());
		return FALSE;
	}
	return TRUE;
}
void dispatcher_handler(u_char *, const struct pcap_pkthdr *, const u_char *);
void InterpreteUDP(u_char *, const struct pcap_pkthdr *, const u_char *);
void PrintPayload(const struct pcap_pkthdr *header, const u_char *packet);


void HeadReader()
{
	pcap_t *fp;
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int inum;
	int i = 0;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct timeval st_ts;
	u_int netmask;
	struct bpf_program fcode;

	/* Load Npcap and its functions. */
	if (!LoadNpcapDlls())
	{
		fprintf(stderr, "Couldn't load Npcap\n");
		exit(1);
	}

	/* Retrieve the device list */
	if (pcap_findalldevs_ex((char*)PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
	{
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}

	/* Print the list */
	for (d = alldevs; d; d = d->next)
	{
		printf("%d. %s", ++i, d->name);
		if (d->description)
			printf(" (%s)\n", d->description);
		else
			printf(" (No description available)\n");
	}

	if (i == 0)
	{
		printf("\nNo interfaces found! Make sure Npcap is installed.\n");
		return;
	}

	printf("Enter the interface number (1-%d):", i);
	std::cin >> inum;

	if (inum < 1 || inum > i)
	{
		printf("\nInterface number out of range.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return;
	}

	/* Jump to the selected adapter */
	for (d = alldevs, i = 0; i< inum - 1;d = d->next, i++);

	/* Open the output adapter */
	if ((fp = pcap_open(d->name, 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf)) == NULL)
	{
		fprintf(stderr, "\nUnable to open adapter %s.\n", errbuf);
		return;
	}
	//fp = pcap_create(d->name,errbuf);
	if (fp == NULL)
	{
		fprintf(stderr, "Failed to create pcap\n");
		return;
	}
	/*pcap_set_snaplen(fp, 100);
	pcap_set_promisc(fp, PCAP_OPENFLAG_PROMISCUOUS);
	pcap_set_timeout(fp, 1000);
	pcap_set_buffer_size(fp, 32768);*/

	///*get a list of time stamp types supported by a capture device*/

	/*int tstamp_types_num=0;
	int* tstamp_typesp;
	if ((tstamp_types_num = pcap_list_tstamp_types(fp, &tstamp_typesp)) == PCAP_ERROR)
	{
		fprintf(stderr, "\nUnable to get time stamp types %s.\n", errbuf);
		return;
	}

	if (tstamp_types_num!=0)
	for (int i = 0;i < tstamp_types_num;i++)
	{
		printf("support time stamp %s\n", pcap_tstamp_type_val_to_name(tstamp_typesp[i]));
	}*/

	
	/*if (pcap_activate(fp) == PCAP_ERROR)
	{
		fprintf(stderr, "\nUnable to activate\n");
		return;
	}*/
	/* Don't care about netmask, it won't be used for this filter */
	netmask = 0xffffff;

	//compile the filter
	if (pcap_compile(fp, &fcode, "tcp", 1, netmask) <0)
	{
		fprintf(stderr, "\nUnable to compile the packet filter. Check the syntax.\n");
		/* Free the device list */
		return;
	}

	//set the filter
	if (pcap_setfilter(fp, &fcode)<0)
	{
		fprintf(stderr, "\nError setting the filter.\n");
		pcap_close(fp);
		/* Free the device list */
		return;
	}

	/* Put the interface in statstics mode */
	if (pcap_setmode(fp, MODE_CAPT)<0)
	{
		fprintf(stderr, "\nError setting the mode.\n");
		pcap_close(fp);
		/* Free the device list */
		return;
	}

	/* At this point, we don't need any more the device list. Free it */
	pcap_freealldevs(alldevs);

	/* Start the main loop */
	pcap_loop(fp, 0, InterpreteUDP, (PUCHAR)&st_ts);

	pcap_close(fp);
	pcap_freecode(&fcode);
	return;
}

void dispatcher_handler(u_char *state, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	struct timeval *old_ts = (struct timeval *)state;
	u_int delay;
	LARGE_INTEGER Bps, Pps;
	struct tm ltime;
	char timestr[16];
	time_t local_tv_sec;
	std::string info((char*)pkt_data);

	/* Calculate the delay in microseconds from the last sample. */
	/* This value is obtained from the timestamp that the associated with the sample. */
	delay = (header->ts.tv_sec - old_ts->tv_sec) * 1000000 + header->ts.tv_usec - old_ts->tv_usec;
	/* Get the number of Bits per second */
	Bps.QuadPart = (((*(LONGLONG*)(pkt_data + 8)) * 8 * 1000000) / (delay));
	/*                                              ^      ^
                                                   	|      |
	                                                |      |
	                                                |      |
	                       converts bytes in bits --       |
	                                                       |
	                  delay is expressed in microseconds --
	*/

	/* Get the number of Packets per second */
	Pps.QuadPart = (((*(LONGLONG*)(pkt_data)) * 1000000) / (delay));

	/* Convert the timestamp to readable format */
	local_tv_sec = header->ts.tv_sec;
	localtime_s(&ltime, &local_tv_sec);
	strftime(timestr, sizeof timestr, "%H:%M:%S", &ltime);

	/* Print timestamp*/
	printf("%s, delay %d ", timestr,delay);

	/* Print the samples */
	printf("BPS=%I64u ", Bps.QuadPart);
	printf("PPS=%I64u\n", Pps.QuadPart);

	//store current timestamp
	old_ts->tv_sec = header->ts.tv_sec;
	old_ts->tv_usec = header->ts.tv_usec;
}

/* Callback function invoked by libpcap for every incoming UDP packet */
void InterpreteUDP(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	struct tm ltime;
	char timestr[16];
	ip_header *ih;
	udp_header *uh;
	u_int ip_len;
	u_short sport, dport;
	time_t local_tv_sec;
	char* payload;

	/*
	* Unused variable
	*/
	(VOID)(param);

	/* convert the timestamp to readable format */
	local_tv_sec = header->ts.tv_sec;
	localtime_s(&ltime, &local_tv_sec);
	strftime(timestr, sizeof timestr, "%H:%M:%S", &ltime);

	/* print timestamp and length of the packet */
	printf("%s.%.6d len:%d ", timestr, header->ts.tv_usec, header->len);
	//printf("%d:%d with length %d", header->ts.tv_sec, header->ts.tv_usec, header->len);

	/* retireve the position of the ip header */
	ih = (ip_header *)(pkt_data +SIZE_ETHERNET); 

	/* retireve the position of the udp header */
	ip_len = (ih->ver_ihl & 0xf) * 4;
	uh = (udp_header *)((u_char*)ih + ip_len);

	/* convert from network byte order to host byte order */
	sport = ntohs(uh->sport);
	dport = ntohs(uh->dport);

	/* print ip addresses and udp ports */
	printf("%d.%d.%d.%d.%d -> %d.%d.%d.%d.%d\n",
		ih->ip_src.S_un.S_un_b.s_b1, //->saddr.byte1
		ih->ip_src.S_un.S_un_b.s_b2,
		ih->ip_src.S_un.S_un_b.s_b3,
		ih->ip_src.S_un.S_un_b.s_b2,
		sport,
		ih->ip_dst.S_un.S_un_b.s_b1,
		ih->ip_dst.S_un.S_un_b.s_b2,
		ih->ip_dst.S_un.S_un_b.s_b3,
		ih->ip_dst.S_un.S_un_b.s_b4,
		dport);

	/*retrieve the packet data*/
	payload = (char*)(pkt_data + 14 + ip_len + SIZE_UDP);

	/* print UDP packet in hex format*/
	PrintPayload(header, pkt_data);
}

/* Callback function invoked by libpcap for every incoming TCP packet */
void InterpreteTCP(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	const struct ethernet_header *ethernet; /* The ethernet header */
	const struct ip_header *ih; /* The IP header */
	const struct tcp_header *tcp; /* The TCP header */
	u_short sport, dport;
	const char *payload; /* Packet payload */

	u_int size_ip;
	u_int size_tcp;

	ethernet = (struct ethernet_header*)(pkt_data);
	ih = (struct ip_header*)(pkt_data + SIZE_ETHERNET);

	size_ip = (ih->ver_ihl) & 0x0f * 4;
	if (size_ip < 20) {
		printf("   * Invalid IP header length: %u bytes\n", size_ip);
		return;
	}
	tcp = (struct tcp_header*)(pkt_data + SIZE_ETHERNET + size_ip);

	/* convert from network byte order to host byte order */
	sport = ntohs(tcp->th_sport);
	dport = ntohs(tcp->th_dport);

	/* print ip addresses and TCP ports */
	printf("%d.%d.%d.%d.%d -> %d.%d.%d.%d.%d\n",
		ih->ip_src.S_un.S_un_b.s_b1, //->saddr.byte1
		ih->ip_src.S_un.S_un_b.s_b2,
		ih->ip_src.S_un.S_un_b.s_b3,
		ih->ip_src.S_un.S_un_b.s_b2,
		sport,
		ih->ip_dst.S_un.S_un_b.s_b1,
		ih->ip_dst.S_un.S_un_b.s_b2,
		ih->ip_dst.S_un.S_un_b.s_b3,
		ih->ip_dst.S_un.S_un_b.s_b4,
		dport);

	size_tcp = TH_OFF(tcp) * 4;
	if (size_tcp < 20) {
		printf("   * Invalid TCP header length: %u bytes\n", size_tcp);
		return;
	}

	/*retrieve the packet data*/
	payload = (char *)(pkt_data + SIZE_ETHERNET + size_ip + size_tcp);

	/*print TCP packet in hex packet*/
	PrintPayload(header, pkt_data);
}

void PrintPayload(const struct pcap_pkthdr *header, const u_char *packet)
{

	ULONG	i, j, ulLines, ulen; //ulBytesReceived;
	char	*pChar, *pLine, *base;
	char	*buf;
	u_int off = 0;
	u_int tlen, tlen1;
	struct pcap_pkthdr *hdr;

	//ulBytesReceived = strlen((char*)header)+strlen((char*)packet);

	buf = (char*)packet;

	off = 0;

	//while (off<ulBytesReceived) {
	tlen1 = header->len;
	tlen = header->caplen;
	printf("Packet length, captured portion: %ld, %ld\n", tlen1, tlen);
	//off += strlen((char*)header);

	ulLines = (tlen + 15) / 16;

	pChar = (char*)(buf + off);
	base = pChar;
	off = ((off + tlen) + (sizeof(int) - 1))&~(sizeof(int) - 1);

	for (i = 0; i < ulLines; i++)
	{

		pLine = pChar;

		printf("%p : ", (void *)(pChar - base));

		ulen = tlen;
		ulen = (ulen > 16) ? 16 : ulen;
		tlen -= ulen;

		for (j = 0; j < ulen; j++)
			printf("%02x ", *(BYTE *)pChar++);

		if (ulen < 16)
			printf("%*s", (16 - ulen) * 3, " ");

		pChar = pLine;

		for (j = 0; j < ulen; j++, pChar++)
			printf("%c", isprint((unsigned char)*pChar) ? *pChar : '.');

		printf("\n");
	}

	printf("\n");
	//}
}

