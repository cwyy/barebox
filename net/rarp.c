/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <clock.h>
#include <net.h>
#include "nfs.h"
#include "rarp.h"
#include "tftp.h"

#define TIMEOUT		5		/* Seconds before trying BOOTP again */
#ifndef	CONFIG_NET_RETRY_COUNT
# define TIMEOUT_COUNT	5		/* # of timeouts before giving up  */
#else
# define TIMEOUT_COUNT  (CONFIG_NET_RETRY_COUNT)
#endif


int		RarpTry;

/*
 *	Handle a RARP received packet.
 */
static void
RarpHandler(uchar * dummi0, unsigned dummi1, unsigned dummi2, unsigned dummi3)
{
#ifdef	DEBUG
	puts ("Got good RARP\n");
#endif
	NetState = NETLOOP_SUCCESS;
}


/*
 *	Timeout on BOOTP request.
 */
static void
RarpTimeout(void)
{
	NetSetTimeout (TIMEOUT * SECOND, RarpTimeout);
	RarpRequest ();
}


void
RarpRequest (void)
{
	int i;
	uchar *pkt;
	ARP_t *	rarp;

	NetOurIP = 0;
	RarpTry = 0;

	printf("RARP broadcast %d\n", ++RarpTry);
	pkt = NetTxPacket;

	pkt += NetSetEther(pkt, NetBcastAddr, PROT_RARP);

	rarp = (ARP_t *)pkt;

	rarp->ar_hrd = htons (ARP_ETHER);
	rarp->ar_pro = htons (PROT_IP);
	rarp->ar_hln = 6;
	rarp->ar_pln = 4;
	rarp->ar_op  = htons (RARPOP_REQUEST);
	memcpy (&rarp->ar_data[0],  NetOurEther, 6);	/* source ET addr */
	memcpy (&rarp->ar_data[6],  &NetOurIP,   4);	/* source IP addr */
	memcpy (&rarp->ar_data[10], NetOurEther, 6);	/* dest ET addr = source ET addr ??*/
	/* dest. IP addr set to broadcast */
	for (i = 0; i <= 3; i++) {
		rarp->ar_data[16 + i] = 0xff;
	}

	NetSendPacket(NetTxPacket, (pkt - NetTxPacket) + ARP_HDR_SIZE);

	NetSetTimeout(TIMEOUT * SECOND, RarpTimeout);
	NetSetHandler(RarpHandler);
}
