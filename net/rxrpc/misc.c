/* Miscellaneous bits
 *
 * Copyright (C) 2016 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licence
 * as published by the Free Software Foundation; either version
 * 2 of the Licence, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <net/sock.h>
#include <net/af_rxrpc.h>
#include "ar-internal.h"

/*
 * The maximum listening backlog queue size that may be set on a socket by
 * listen().
 */
unsigned int rxrpc_max_backlog __read_mostly = 10;

/*
 * How long to wait before scheduling ACK generation after seeing a
 * packet with RXRPC_REQUEST_ACK set (in jiffies).
 */
unsigned int rxrpc_requested_ack_delay = 1;

/*
 * How long to wait before scheduling an ACK with subtype DELAY (in jiffies).
 *
 * We use this when we've received new data packets.  If those packets aren't
 * all consumed within this time we will send a DELAY ACK if an ACK was not
 * requested to let the sender know it doesn't need to resend.
 */
unsigned int rxrpc_soft_ack_delay = 1 * HZ;

/*
 * How long to wait before scheduling an ACK with subtype IDLE (in jiffies).
 *
 * We use this when we've consumed some previously soft-ACK'd packets when
 * further packets aren't immediately received to decide when to send an IDLE
 * ACK let the other end know that it can free up its Tx buffer space.
 */
unsigned int rxrpc_idle_ack_delay = 0.5 * HZ;

/*
 * Receive window size in packets.  This indicates the maximum number of
 * unconsumed received packets we're willing to retain in memory.  Once this
 * limit is hit, we should generate an EXCEEDS_WINDOW ACK and discard further
 * packets.
 */
unsigned int rxrpc_rx_window_size = RXRPC_INIT_RX_WINDOW_SIZE;
#if (RXRPC_RXTX_BUFF_SIZE - 1) < RXRPC_INIT_RX_WINDOW_SIZE
#error Need to reduce RXRPC_INIT_RX_WINDOW_SIZE
#endif

/*
 * Maximum Rx MTU size.  This indicates to the sender the size of jumbo packet
 * made by gluing normal packets together that we're willing to handle.
 */
unsigned int rxrpc_rx_mtu = 5692;

/*
 * The maximum number of fragments in a received jumbo packet that we tell the
 * sender that we're willing to handle.
 */
unsigned int rxrpc_rx_jumbo_max = 4;

/*
 * Time till packet resend (in milliseconds).
 */
unsigned int rxrpc_resend_timeout = 4 * 1000;

const char *const rxrpc_pkts[] = {
	"?00",
	"DATA", "ACK", "BUSY", "ABORT", "ACKALL", "CHALL", "RESP", "DEBUG",
	"?09", "?10", "?11", "?12", "VERSION", "?14", "?15"
};

const s8 rxrpc_ack_priority[] = {
	[0]				= 0,
	[RXRPC_ACK_DELAY]		= 1,
	[RXRPC_ACK_REQUESTED]		= 2,
	[RXRPC_ACK_IDLE]		= 3,
	[RXRPC_ACK_DUPLICATE]		= 4,
	[RXRPC_ACK_OUT_OF_SEQUENCE]	= 5,
	[RXRPC_ACK_EXCEEDS_WINDOW]	= 6,
	[RXRPC_ACK_NOSPACE]		= 7,
	[RXRPC_ACK_PING_RESPONSE]	= 8,
	[RXRPC_ACK_PING]		= 9,
};

const char *rxrpc_acks(u8 reason)
{
	static const char *const str[] = {
		"---", "REQ", "DUP", "OOS", "WIN", "MEM", "PNG", "PNR", "DLY",
		"IDL", "-?-"
	};

	if (reason >= ARRAY_SIZE(str))
		reason = ARRAY_SIZE(str) - 1;
	return str[reason];
}

const char rxrpc_skb_traces[rxrpc_skb__nr_trace][7] = {
	[rxrpc_skb_rx_cleaned]		= "Rx CLN",
	[rxrpc_skb_rx_freed]		= "Rx FRE",
	[rxrpc_skb_rx_got]		= "Rx GOT",
	[rxrpc_skb_rx_lost]		= "Rx *L*",
	[rxrpc_skb_rx_received]		= "Rx RCV",
	[rxrpc_skb_rx_purged]		= "Rx PUR",
	[rxrpc_skb_rx_rotated]		= "Rx ROT",
	[rxrpc_skb_rx_seen]		= "Rx SEE",
	[rxrpc_skb_tx_cleaned]		= "Tx CLN",
	[rxrpc_skb_tx_freed]		= "Tx FRE",
	[rxrpc_skb_tx_got]		= "Tx GOT",
	[rxrpc_skb_tx_lost]		= "Tx *L*",
	[rxrpc_skb_tx_new]		= "Tx NEW",
	[rxrpc_skb_tx_rotated]		= "Tx ROT",
	[rxrpc_skb_tx_seen]		= "Tx SEE",
};

const char rxrpc_conn_traces[rxrpc_conn__nr_trace][4] = {
	[rxrpc_conn_new_client]		= "NWc",
	[rxrpc_conn_new_service]	= "NWs",
	[rxrpc_conn_queued]		= "QUE",
	[rxrpc_conn_seen]		= "SEE",
	[rxrpc_conn_got]		= "GOT",
	[rxrpc_conn_put_client]		= "PTc",
	[rxrpc_conn_put_service]	= "PTs",
};

const char rxrpc_client_traces[rxrpc_client__nr_trace][7] = {
	[rxrpc_client_activate_chans]	= "Activa",
	[rxrpc_client_alloc]		= "Alloc ",
	[rxrpc_client_chan_activate]	= "ChActv",
	[rxrpc_client_chan_disconnect]	= "ChDisc",
	[rxrpc_client_chan_pass]	= "ChPass",
	[rxrpc_client_chan_unstarted]	= "ChUnst",
	[rxrpc_client_cleanup]		= "Clean ",
	[rxrpc_client_count]		= "Count ",
	[rxrpc_client_discard]		= "Discar",
	[rxrpc_client_duplicate]	= "Duplic",
	[rxrpc_client_exposed]		= "Expose",
	[rxrpc_client_replace]		= "Replac",
	[rxrpc_client_to_active]	= "->Actv",
	[rxrpc_client_to_culled]	= "->Cull",
	[rxrpc_client_to_idle]		= "->Idle",
	[rxrpc_client_to_inactive]	= "->Inac",
	[rxrpc_client_to_waiting]	= "->Wait",
	[rxrpc_client_uncount]		= "Uncoun",
};

const char rxrpc_transmit_traces[rxrpc_transmit__nr_trace][4] = {
	[rxrpc_transmit_wait]		= "WAI",
	[rxrpc_transmit_queue]		= "QUE",
	[rxrpc_transmit_queue_reqack]	= "QRA",
	[rxrpc_transmit_queue_last]	= "QLS",
	[rxrpc_transmit_rotate]		= "ROT",
	[rxrpc_transmit_end]		= "END",
};

const char rxrpc_receive_traces[rxrpc_receive__nr_trace][4] = {
	[rxrpc_receive_incoming]	= "INC",
	[rxrpc_receive_queue]		= "QUE",
	[rxrpc_receive_queue_last]	= "QLS",
	[rxrpc_receive_front]		= "FRN",
	[rxrpc_receive_rotate]		= "ROT",
	[rxrpc_receive_end]		= "END",
};

const char rxrpc_recvmsg_traces[rxrpc_recvmsg__nr_trace][5] = {
	[rxrpc_recvmsg_enter]		= "ENTR",
	[rxrpc_recvmsg_wait]		= "WAIT",
	[rxrpc_recvmsg_dequeue]		= "DEQU",
	[rxrpc_recvmsg_hole]		= "HOLE",
	[rxrpc_recvmsg_next]		= "NEXT",
	[rxrpc_recvmsg_cont]		= "CONT",
	[rxrpc_recvmsg_full]		= "FULL",
	[rxrpc_recvmsg_data_return]	= "DATA",
	[rxrpc_recvmsg_terminal]	= "TERM",
	[rxrpc_recvmsg_to_be_accepted]	= "TBAC",
	[rxrpc_recvmsg_return]		= "RETN",
};

const char rxrpc_rtt_tx_traces[rxrpc_rtt_tx__nr_trace][5] = {
	[rxrpc_rtt_tx_ping]		= "PING",
	[rxrpc_rtt_tx_data]		= "DATA",
};

const char rxrpc_rtt_rx_traces[rxrpc_rtt_rx__nr_trace][5] = {
	[rxrpc_rtt_rx_ping_response]	= "PONG",
	[rxrpc_rtt_rx_requested_ack]	= "RACK",
};
