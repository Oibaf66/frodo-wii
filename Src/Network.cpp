/*
 *  Network.h - Network handling
 *
 *  Frodo (C) 2009 Simon Kagstrom
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sysdeps.h"
#include "Network.h"
#include "Display.h"
#include "Prefs.h"
#include "main.h"
#include "C64.h"

#include "utils.hh"
#include "data_store.hh"
#include "gui/gui.hh"
#include "gui/status_bar.hh"
#include "gui/network_user_menu.hh"

#if defined(GEKKO)
# include <wiiuse/wpad.h>
#endif

#define N_SQUARES_W 16
#define N_SQUARES_H 8

#define SQUARE_W (DISPLAY_X / N_SQUARES_W)
#define SQUARE_H (DISPLAY_Y / N_SQUARES_H)

#define SQUARE_TO_X(square) ( ((square) % N_SQUARES_W) * SQUARE_W )
#define SQUARE_TO_Y(square) ( ((square) / N_SQUARES_W) * SQUARE_H )

/* Worst cases for RLE and DIFF */
#define RAW_SIZE  ( (SQUARE_W * SQUARE_H) / 2 )
#define RLE_SIZE  ( RAW_SIZE * 4 + 8)
#define DIFF_SIZE ( RAW_SIZE * 4 + 8)

Network::Network(const char *remote_host, int port)
{
	const size_t size = NETWORK_UPDATE_SIZE;

	this->InitNetwork();

	this->is_master = true; /* Assume true */
	this->connected = false;

	/* "big enough" buffer */
	this->ud = (NetworkUpdate*)malloc( size );
	assert(this->ud);

	this->ResetNetworkUpdate();
	this->traffic = 0;
	this->last_traffic = 0;
	this->target_kbps = 160000; /* kilobit per seconds */
	this->kbps = 0;

	this->raw_buf = (Uint8*)malloc(RAW_SIZE);
	this->rle_buf = (Uint8*)malloc(RLE_SIZE);
	this->diff_buf = (Uint8*)malloc(DIFF_SIZE);
	assert(this->raw_buf && this->rle_buf && this->diff_buf);

	/* Go from lower right to upper left */
	this->refresh_square = N_SQUARES_W * N_SQUARES_H - 1;
	this->square_updated = (Uint32*)malloc( N_SQUARES_W * N_SQUARES_H * sizeof(Uint32));
	assert(this->square_updated);
	memset(this->square_updated, 0, N_SQUARES_W * N_SQUARES_H * sizeof(Uint32));

	this->screen = (Uint8 *)malloc(DISPLAY_X * DISPLAY_Y);
	assert(this->screen);

	this->sound_head = this->sound_tail = 0;
	this->sound_last_cycles = SDL_GetTicks();
	memset(this->sound_active, 0, sizeof(this->sound_active));

	/* Assume black screen */
	memset(this->screen, 0, DISPLAY_X * DISPLAY_Y);
	memset(this->screenshot, 0, sizeof(this->screenshot));

	Network::networking_started = true;
	this->peer_selected = -1;
	/* Peer addresses, if it fails we are out of luck */
	panic_if (this->InitSocket() == false,
			"Could not init the socket\n");

	/* Setup the socket addresses */
	memset(&this->peer_addr, 0, sizeof(this->peer_addr));
	memset(&this->server_addr, 0, sizeof(this->server_addr));
	panic_if (this->InitSockaddr(&this->server_addr, remote_host, port) == false,
			"Can't initialize socket address to server\n");

	this->network_connection_state = CONN_CONNECT_TO_BROKER;
	this->connection_error_message = "Connection OK";
}

Network::~Network()
{
	free(this->ud);
	free(this->square_updated);
	free(this->raw_buf);
	free(this->rle_buf);
	free(this->diff_buf);
	free(this->screen);

	this->CloseSocket();
	this->ShutdownNetwork();
}

void Network::Tick(int ms)
{
	int last_kbps = ((this->traffic - this->last_traffic) * 8) * (1000 / ms);

	/* 1/3 of the new value, 2/3 of the old */
	this->kbps = 2 * (this->kbps / 3) + (last_kbps / 3);
	this->last_traffic = this->traffic;
}

bool Network::DecodeDisplayDiff(struct NetworkUpdate *src,
		int x_start, int y_start)
{
	struct NetworkUpdateDisplay *dp = (struct NetworkUpdateDisplay *)src->data;
	int p = 0;
	int x = x_start;
	int y = y_start;
	int sz = src->size - sizeof(NetworkUpdate) - sizeof(NetworkUpdateDisplay);

	/* Something is wrong if this is true... */
	if (sz % 2 != 0)
		return false;

	while (p < sz)
	{
		Uint8 len = dp->data[p];
		Uint8 color = dp->data[p+1];
		int x_diff = (x - x_start + len) % SQUARE_W;
		int y_diff = (x - x_start + len) / SQUARE_W;

		x = x_start + x_diff;
		y = y + y_diff;
		this->screen[y * DISPLAY_X + x] = color;
		p += 2;
	}

	return true;
}

bool Network::DecodeDisplayRLE(struct NetworkUpdate *src,
		int x_start, int y_start)
{
	struct NetworkUpdateDisplay *dp = (struct NetworkUpdateDisplay *)src->data;
	int p = 0;
	int x = x_start;
	int y = y_start;
	int sz = src->size - sizeof(NetworkUpdate) - sizeof(NetworkUpdateDisplay);

	/* Something is wrong if this is true... */
	if (sz % 2 != 0)
		return false;

	while (p < sz)
	{
		Uint8 len = dp->data[p];
		Uint8 color = dp->data[p+1];

		while (len > 0)
		{
			this->screen[y * DISPLAY_X + x] = color;
			len--;
			x++;
			if ((x - x_start) % SQUARE_W == 0)
			{
				x = x_start;
				y++;
			}
		}
		p += 2;
	}

	return true;
}

bool Network::DecodeDisplayRaw(struct NetworkUpdate *src,
		int x_start, int y_start)
{
	struct NetworkUpdateDisplay *dp = (struct NetworkUpdateDisplay *)src->data;
	const int raw_w = SQUARE_W / 2;

	for (int y = y_start; y < y_start + SQUARE_H; y++)
	{
		for (int x = x_start; x < x_start + SQUARE_W; x += 2)
		{
			Uint8 v = dp->data[(y - y_start) * raw_w + (x - x_start) / 2];
			Uint8 a = v >> 4;
			Uint8 b = v & 0xf;

			this->screen[ y * DISPLAY_X + x ] = a;
			this->screen[ y * DISPLAY_X + x + 1 ] = b;
		}
	}

	return true;
}

bool Network::CompareSquare(Uint8 *a, Uint8 *b)
{
	for (int y = 0; y < SQUARE_H; y++)
	{
		for (int x = 0; x < SQUARE_W; x += 4)
		{
			Uint32 va = *((Uint32*)&a[ y * DISPLAY_X + x ]);
			Uint32 vb = *((Uint32*)&b[ y * DISPLAY_X + x ]);

			if (va != vb)
				return false;
		}
	}

	return true;
}

void Network::EncodeScreenshot(Uint8 *dst, Uint8 *master)
{
	int x, y;
	int cnt = 0;
	int p = 0;

	memset(dst, 0, (SCREENSHOT_X * SCREENSHOT_Y) / 2);
	for (y = 0; y < DISPLAY_Y; y += SCREENSHOT_FACTOR)
	{
		for (x = 0; x < DISPLAY_X; x += SCREENSHOT_FACTOR)
		{
			Uint8 col_s = master[ y * DISPLAY_X + x ];
			bool is_odd = (cnt & 1) == 1;
			int raw_shift = (is_odd ? 0 : 4);

			/* Every second is shifted */
			dst[ p ] |=	(col_s << raw_shift);
			if (is_odd)
				p++;
			cnt++;
		}
	}
}

void Network::EncodeDisplay(Uint8 *master, Uint8 *remote)
{
	if (!this->network_connection_state == MASTER)
		return;
	for ( int sq = 0; sq < N_SQUARES_H * N_SQUARES_W; sq++ )
	{
		Uint8 *p_master = &master[ SQUARE_TO_Y(sq) * DISPLAY_X + SQUARE_TO_X(sq) ]; 
		Uint8 *p_remote = &remote[ SQUARE_TO_Y(sq) * DISPLAY_X + SQUARE_TO_X(sq) ]; 

		/* Refresh periodically or if the squares differ */
		if ( (this->refresh_square == sq && this->kbps < this->target_kbps * 0.7) ||
				this->CompareSquare(p_master, p_remote) == false)
		{
			NetworkUpdate *dst = (NetworkUpdate *)this->cur_ud;

			/* Updated, encode this */
			this->EncodeDisplaySquare(dst, master, remote, sq,
					this->refresh_square != sq);
			this->AddNetworkUpdate(dst);

			/* This has been refreshed, move to the next one */
			if (this->refresh_square == sq)
			{
				this->refresh_square--;
				if (this->refresh_square < 0)
					this->refresh_square = N_SQUARES_H * N_SQUARES_W - 1;
			}
		}
		else
			this->square_updated[sq] = 0;
	}
	memcpy(remote, master, DISPLAY_X * DISPLAY_Y);
}


size_t Network::EncodeDisplaySquare(struct NetworkUpdate *dst,
		Uint8 *screen, Uint8 *remote, int square,
		bool use_diff)
{
	struct NetworkUpdateDisplay *dp = (struct NetworkUpdateDisplay *)dst->data;
	const int x_start = SQUARE_TO_X(square);
	const int y_start = SQUARE_TO_Y(square);
	Uint8 rle_color = screen[ y_start * DISPLAY_X + x_start ];
	int rle_len = 0, diff_len = 0;
	size_t rle_sz = 0, diff_sz = 0;
	const int raw_w = SQUARE_W / 2;
	int type = DISPLAY_UPDATE_RAW;
	size_t out;

	for (int y = y_start; y < y_start + SQUARE_H; y++)
	{
		memset( &this->raw_buf[(y - y_start) * raw_w], 0, raw_w );

		for (int x = x_start; x < x_start + SQUARE_W; x++)
		{
			Uint8 col_s = screen[ y * DISPLAY_X + x ];
			Uint8 col_r = remote[ y * DISPLAY_X + x ];
			bool is_odd = (x & 1) == 1;
			int raw_shift = (is_odd ? 0 : 4);

			/* Every second is shifted */
			this->raw_buf[ (y - y_start) * raw_w + (x - x_start) / 2 ] |=
				(col_s << raw_shift);

			if (rle_color != col_s ||
					rle_len >= 255)
			{
				this->rle_buf[rle_sz] = rle_len;
				this->rle_buf[rle_sz + 1] = rle_color;
				rle_sz += 2;

				rle_len = 0;
				rle_color = col_s;
			}

			if (col_r != col_s || diff_len >= 255)
			{
				this->diff_buf[diff_sz] = diff_len;
				this->diff_buf[diff_sz + 1] = col_s;
				diff_sz += 2;
				diff_len = 0;
			}

			diff_len++;
			rle_len++;
		}
	}

	/* The last section for RLE */
	if (rle_len != 0)
	{
		this->rle_buf[rle_sz] = rle_len;
		this->rle_buf[rle_sz + 1] = rle_color;

		rle_sz += 2;
	}

	out = RAW_SIZE;
	if (use_diff && (diff_sz < rle_sz && diff_sz < RAW_SIZE))
	{
		memcpy(dp->data, this->diff_buf, diff_sz);
		type = DISPLAY_UPDATE_DIFF;
		out = diff_sz;
	}
	else if (rle_sz < RAW_SIZE)
	{
		memcpy(dp->data, this->rle_buf, rle_sz);
		type = DISPLAY_UPDATE_RLE;
		out = rle_sz;
	}		
	else
		memcpy(dp->data, this->raw_buf, RAW_SIZE);

	/* Setup the structure */
	dp->square = square;
	dst = InitNetworkUpdate(dst, type,
			sizeof(struct NetworkUpdate) + sizeof(struct NetworkUpdateDisplay) + out);
	this->square_updated[square] = out | (type << 16);

	return dst->size;
}

bool Network::DecodeDisplayUpdate(struct NetworkUpdate *src)
{
	struct NetworkUpdateDisplay *dp = (struct NetworkUpdateDisplay *)src->data;
	int square = dp->square;
	const int square_x = SQUARE_TO_X(square);
	const int square_y = SQUARE_TO_Y(square);

	if (src->type == DISPLAY_UPDATE_DIFF)
		return this->DecodeDisplayDiff(src, square_x, square_y);
	else if (src->type == DISPLAY_UPDATE_RAW)
		return this->DecodeDisplayRaw(src, square_x, square_y);
	else if (src->type == DISPLAY_UPDATE_RLE)
		return this->DecodeDisplayRLE(src, square_x, square_y);

	/* Error */
	return false;
}

void Network::EncodeTextMessage(char *str)
{
	NetworkUpdate *dst = (NetworkUpdate *)this->cur_ud;
	char *p = (char*)dst->data;
	size_t len = strlen(str) + 1;

	len += (len & 3);
	dst = InitNetworkUpdate(dst, TEXT_MESSAGE,
			sizeof(NetworkUpdate) + len);
	memset(p, 0, len);
	strncpy(p, str, len - 1);

	this->AddNetworkUpdate(dst);
}


void Network::EnqueueSound(uint32 linecnt_diff, uint8 adr, uint8 val)
{
	NetworkUpdateSoundInfo *cur = &this->sound_active[this->sound_head];

	cur->adr = adr;
	cur->val = val;
	cur->delay_cycles = linecnt_diff;

	this->sound_head++;

	if (this->sound_head >= NETWORK_SOUND_BUF_SIZE)
		this->sound_head = 0;

	/* Head has reached tail */
	if (this->sound_head == this->sound_tail)
		this->sound_tail = (this->sound_head + 1) % NETWORK_SOUND_BUF_SIZE;
}

void Network::RegisterSidWrite(uint32 linecnt, uint8 adr, uint8 val)
{
	this->EnqueueSound(linecnt - this->sound_last_cycles, adr, val);

	/* Update the cycle counter */
	sound_last_cycles = linecnt;
}

void Network::FlushSound(void)
{
	NetworkUpdate *dst = this->cur_ud;
	NetworkUpdateSound *snd = (NetworkUpdateSound *)dst->data;
	NetworkUpdateSoundInfo *snd_info = snd->info;

	snd->flags = 0;
	snd->n_items = this->sound_head - this->sound_tail;

	if (this->sound_head < this->sound_tail) {
		snd->n_items = NETWORK_SOUND_BUF_SIZE - this->sound_tail + this->sound_head;
		memcpy(snd_info, &this->sound_active[this->sound_tail],
				(NETWORK_SOUND_BUF_SIZE - this->sound_tail) * sizeof(struct NetworkUpdateSoundInfo));
		memcpy(snd_info + NETWORK_SOUND_BUF_SIZE - this->sound_tail,
				&this->sound_active[0],
				this->sound_head * sizeof(struct NetworkUpdateSoundInfo));
	}
	else
	{
		memcpy(snd_info, &this->sound_active[this->sound_tail],
				(this->sound_head - this->sound_tail) * sizeof(struct NetworkUpdateSoundInfo));
	}
	this->sound_tail = this->sound_head;

	this->sound_last_send = SDL_GetTicks();

	InitNetworkUpdate(dst, SOUND_UPDATE, sizeof(NetworkUpdate) +
			sizeof(NetworkUpdateSound) + sizeof(NetworkUpdateSoundInfo) * snd->n_items);
	this->AddNetworkUpdate(dst);
	this->sound_last_cycles = TheC64->linecnt;
}

struct NetworkUpdateSoundInfo *Network::DequeueSound()
{
	struct NetworkUpdateSoundInfo *out;

	if (this->sound_tail == this->sound_head)
		return NULL;
	out = &this->sound_active[this->sound_tail];
	this->sound_tail = (this->sound_tail + 1) % NETWORK_SOUND_BUF_SIZE;

	return out;
}

void Network::EncodeJoystickUpdate(Uint8 v)
{
	struct NetworkUpdate *dst = this->cur_ud;
	struct NetworkUpdateJoystick *j = (NetworkUpdateJoystick *)dst->data;

	if (TheC64->network_connection_type == MASTER || this->cur_joystick_data == v)
		return;
	dst = InitNetworkUpdate(dst, JOYSTICK_UPDATE,
			sizeof(NetworkUpdate) + sizeof(NetworkUpdateJoystick));
	j->val = v;

	this->AddNetworkUpdate(dst);
	this->cur_joystick_data = v;
}

void Network::ResetNetworkUpdate(void)
{
	memset(this->ud, 0, NETWORK_UPDATE_SIZE);

	this->cur_ud = InitNetworkUpdate(this->ud, STOP, sizeof(NetworkUpdate));
}

void Network::DrawTransferredBlocks(SDL_Surface *screen)
{
	const int x_border = (DISPLAY_X - FULL_DISPLAY_X / 2);
	const int y_border = (DISPLAY_Y - FULL_DISPLAY_Y / 2);

	for (int sq = 0; sq < N_SQUARES_W * N_SQUARES_H; sq++)
	{
		int x = SQUARE_TO_X(sq) * 2 - x_border;
		int y = SQUARE_TO_Y(sq) * 2 - y_border;
		int w = SQUARE_W * 2;
		int h = SQUARE_H * 2;

		if (this->square_updated[sq])
		{
			SDL_Rect l = {x,     y,     1, h};
			SDL_Rect r = {x + w, y,     1, h};
			SDL_Rect u = {x,     y,     w, 1};
			SDL_Rect d = {x,     y + h, w, 1};
			Uint32 raw = this->square_updated[sq];
			SDL_Rect size = {x,  y,  2 * ((raw & 0xffff) / 17), 4};
			Uint32 color = 4;

			if ((raw >> 16) == DISPLAY_UPDATE_RLE)
				color = 5;
			else if ((raw >> 16) == DISPLAY_UPDATE_DIFF)
				color = 6;

			SDL_FillRect(screen, &l, 19);
			SDL_FillRect(screen, &r, 19);
			SDL_FillRect(screen, &u, 19);
			SDL_FillRect(screen, &d, 19);

			SDL_FillRect(screen, &size, color);
		}
	}
}

bool Network::ReceiveUpdate()
{
	struct timeval tv;

	memset(&tv, 0, sizeof(tv));
	return this->ReceiveUpdate(this->ud, NETWORK_UPDATE_SIZE, &tv);
}

bool Network::ReceiveUpdate(struct timeval *tv)
{
	return this->ReceiveUpdate(this->ud, NETWORK_UPDATE_SIZE, tv);
}

bool Network::ReceiveUpdate(NetworkUpdate *dst, size_t total_sz,
		struct timeval *tv)
{
	Uint8 *p = (Uint8*)dst;
	size_t sz_left = total_sz;
	size_t received = 0;
	bool has_stop = false;

	if (this->Select(this->sock, tv) == false)
		return false;

	if (sz_left <= 0)
		return false;

	/* Receive the header */
	do {
		ssize_t actual_sz = this->ReceiveFrom(p, this->sock,
				4096, NULL);
		if (actual_sz <= 0)
			return false;

		received += actual_sz;
		if (ntohs(dst->magic) != FRODO_NETWORK_MAGIC) {
			printf("Packet with wrong magic received\n");
			return false;
		}
		if (this->ScanDataForStop(dst, received) == true)
			break;

		sz_left -= actual_sz;
		p = p + actual_sz;
	} while (!has_stop);

	if (this->DeMarshalAllData(dst, received) == false) {
		printf("Demarshal error\n");
		return false;
	}

	return true;
}

bool Network::SendUpdate(struct sockaddr_in *addr)
{
	NetworkUpdate *src = this->ud;
	NetworkUpdate *stop = InitNetworkUpdate(this->cur_ud, STOP, sizeof(NetworkUpdate));
	size_t sz;

	/* Nothing to send, that's OK */
	if ( src == stop )
		return true;

	/* Add a stop at the end of the update */
	this->AddNetworkUpdate(stop);

	if (this->MarshalAllData(src) == false)
		return false;

	sz = this->GetNetworkUpdateSize();
	if (sz <= 0)
		return false;
	size_t cur_sz = 0;
	Uint8 *p = (Uint8*)src; 
	do
	{
		size_t size_to_send = this->FillNetworkBuffer((NetworkUpdate*)p);
		ssize_t v;

		v = this->SendTo((void*)p, this->sock,
				size_to_send, addr);
		if (v < 0 || (size_t)v != size_to_send)
			return false;
		cur_sz += size_to_send;
		p += size_to_send;
	} while (cur_sz < sz);
	this->traffic += cur_sz;

	return true;
}

size_t Network::FillNetworkBuffer(NetworkUpdate *cur)
{
	size_t sz = 0;
	size_t cur_sz;
	int cnt = 0;

	while(1)
	{
		cur_sz = ntohl(cur->size);

		if (sz + cur_sz >= 4096)
			break;

		cnt++;
		sz += cur_sz;
		if (ntohs(cur->type) == STOP)
			break;
		cur = (NetworkUpdate*)((Uint8*)cur + cur_sz);
	}
	assert(sz <= 4096);

	return sz;
}


void Network::AddNetworkUpdate(NetworkUpdate *update)
{
	Uint8 *next = (Uint8*)this->cur_ud + update->size;

	this->cur_ud = (NetworkUpdate*)next;
}

bool Network::MarshalData(NetworkUpdate *p)
{
	switch (p->type)
	{
	case DISPLAY_UPDATE_RAW:
	case DISPLAY_UPDATE_RLE:
	case DISPLAY_UPDATE_DIFF:
	case JOYSTICK_UPDATE:
	case DISCONNECT:
	case CONNECT_TO_PEER:
	case TEXT_MESSAGE:
	case STOP:
		break;
	case BANDWIDTH_PING:
	case BANDWIDTH_ACK:
	case PING:
	case ACK:
	{
		NetworkUpdatePingAck *pa = (NetworkUpdatePingAck *)p->data;
		pa->seq = htonl(pa->seq);
	} break;
	case SELECT_PEER:
	{
		NetworkUpdateSelectPeer *sp = (NetworkUpdateSelectPeer *)p->data;
		sp->server_id = htonl(sp->server_id);
	} break;
	case REGISTER_DATA:
	{
		NetworkUpdateRegisterData *ds = (NetworkUpdateRegisterData *)p->data;

		ds->key = htonl(ds->key);
		ds->metadata = htonl(ds->metadata);
	} break;
	case LIST_PEERS:
	{
		NetworkUpdateListPeers *lp = (NetworkUpdateListPeers *)p->data;
		for (unsigned int i = 0; i < lp->n_peers; i++)
		{
			NetworkUpdatePeerInfo *peer = &lp->peers[i];

			peer->key = htons(peer->key);
			peer->private_port = htons(peer->private_port);
			peer->public_port = htons(peer->public_port);
			peer->is_master = htons(peer->is_master);
			peer->server_id = htonl(peer->server_id);
			peer->version = htonl(peer->version);
			peer->avatar = htonl(peer->avatar);
			peer->screenshot_key = htonl(peer->screenshot_key);
		}
		lp->n_peers = htonl(lp->n_peers);
		lp->your_port = htons(lp->your_port);
	} break;
	case CONNECT_TO_BROKER:
	{
		NetworkUpdatePeerInfo *pi = (NetworkUpdatePeerInfo *)p->data;

		/* The rest is simply ignored */
		pi->is_master = htons(pi->is_master);
		pi->key = htons(pi->key);
		pi->version = htonl(pi->version);
	} break;
	case SOUND_UPDATE:
	{
		NetworkUpdateSound *snd = (NetworkUpdateSound *)p->data;
		NetworkUpdateSoundInfo *info = (NetworkUpdateSoundInfo *)snd->info;
		int items = snd->n_items;

		snd->flags = htons(snd->flags);
		snd->n_items = htons(snd->n_items);

		for (int i = 0; i < items; i++)
		{
			NetworkUpdateSoundInfo *cur = &info[i];

			cur->delay_cycles = htons(cur->delay_cycles);
		}
	} break;
	default:
		/* Unknown data... */
		fprintf(stderr, "Got unknown data %d while marshalling. Something is wrong\n",
				p->type);
		exit(0); // FIXME! TMP!!
		return false;
	}

	p->size = htonl(p->size);
	p->magic = htons(p->magic);
	p->type = htons(p->type);

	return true;
}

bool Network::MarshalAllData(NetworkUpdate *ud)
{
	NetworkUpdate *p = ud;

	while (p->type != STOP)
	{
		NetworkUpdate *nxt = this->GetNext(p);

		if (this->MarshalData(p) == false)
			return false;
		p = nxt;
	}

	/* The stop tag */
	return this->MarshalData(p);
}

bool Network::DeMarshalData(NetworkUpdate *p)
{
	p->size = ntohl(p->size);
	p->magic = ntohs(p->magic);
	p->type = ntohs(p->type);

	if (p->magic != FRODO_NETWORK_MAGIC)
		return false;

	switch (p->type)
	{
	case DISPLAY_UPDATE_RAW:
	case DISPLAY_UPDATE_RLE:
	case DISPLAY_UPDATE_DIFF:
	case JOYSTICK_UPDATE:
	case DISCONNECT:
	case CONNECT_TO_PEER:
	case TEXT_MESSAGE:
	case STOP:
		/* Nothing to do, just bytes */
		break;
	case BANDWIDTH_PING:
	case BANDWIDTH_ACK:
	case PING:
	case ACK:
	{
		NetworkUpdatePingAck *pa = (NetworkUpdatePingAck *)p->data;
		pa->seq = ntohl(pa->seq);
	} break;
	case SELECT_PEER:
	{
		NetworkUpdateSelectPeer *sp = (NetworkUpdateSelectPeer *)p->data;
		sp->server_id = ntohl(sp->server_id);
	} break;
	case REGISTER_DATA:
	{
		NetworkUpdateRegisterData *ds = (NetworkUpdateRegisterData *)p->data;

		ds->key = ntohl(ds->key);
		ds->metadata = ntohl(ds->metadata);
	} break;
	case LIST_PEERS:
	{
		NetworkUpdateListPeers *lp = (NetworkUpdateListPeers *)p->data;

		lp->n_peers = ntohl(lp->n_peers);
		for (unsigned int i = 0; i < lp->n_peers; i++)
		{
			NetworkUpdatePeerInfo *peer = &lp->peers[i];

			peer->key = ntohs(peer->key);
			peer->private_port = ntohs(peer->private_port);
			peer->public_port = ntohs(peer->public_port);
			peer->is_master = ntohs(peer->is_master);
			peer->server_id = ntohl(peer->server_id);
			peer->version = ntohl(peer->version);
			peer->avatar = ntohl(peer->avatar);
			peer->screenshot_key = ntohl(peer->screenshot_key);
		}
		lp->your_port = ntohs(lp->your_port);
	} break;
	case SOUND_UPDATE:
	{
		NetworkUpdateSound *snd = (NetworkUpdateSound *)p->data;
		NetworkUpdateSoundInfo *info = (NetworkUpdateSoundInfo *)snd->info;

		snd->flags = ntohs(snd->flags);
		snd->n_items = ntohs(snd->n_items);
		for (unsigned int i = 0; i < snd->n_items; i++)
		{
			NetworkUpdateSoundInfo *cur = &info[i];

			cur->delay_cycles = ntohs(cur->delay_cycles);
		}
	} break;
	default:
		/* Unknown data... */
		printf("Got unknown data: %d\n", p->type);
		return false;
	}

	return true;
}

bool Network::DeMarshalAllData(NetworkUpdate *ud, size_t max_size)
{
	NetworkUpdate *p = ud;
	int cnt = 0;
	size_t sz = 0;

	while (ntohs(p->type) != STOP &&
			sz + ntohl(p->size) < max_size)
	{
		if (this->DeMarshalData(p) == false)
			return false;
		sz += p->size;
		cnt++;
		p = this->GetNext(p);
	}

	return this->DeMarshalData(p);
}

bool Network::ScanDataForStop(NetworkUpdate *ud, size_t max_size)
{
	NetworkUpdate *p = ud;
	size_t sz = 0;

	while (ntohs(p->type) != STOP &&
			sz + ntohl(p->size) < max_size)
	{
		size_t cur_sz = ntohl(p->size);

		sz += cur_sz;
		p = (NetworkUpdate*)((Uint8*)p + cur_sz);
	}

	/* The stop tag (maybe) */
	return ntohs(p->type) == STOP;
}


bool Network::DecodeUpdate(C64Display *display, uint8 *js, MOS6581 *dst)
{
	NetworkUpdate *p = this->ud;
	bool out = true;

	while (p->type != STOP)
	{
		switch(p->type)
		{
		case SOUND_UPDATE:
		{
			/* No sound updates _to_ the master */
			if (TheC64->network_connection_type == MASTER)
				break;
			NetworkUpdateSound *snd = (NetworkUpdateSound *)p->data;
			NetworkUpdateSoundInfo *info = (NetworkUpdateSoundInfo *)snd->info;

			for (unsigned int i = 0; i < snd->n_items; i++)
			{
				NetworkUpdateSoundInfo *cur = &info[i];

				this->EnqueueSound(cur->delay_cycles, cur->adr, cur->val);
			}
		} break;
		case DISPLAY_UPDATE_RAW:
		case DISPLAY_UPDATE_RLE:
		case DISPLAY_UPDATE_DIFF:
			/* No screen updates _to_ the master */
			if (TheC64->network_connection_type == MASTER)
				break;
			if (this->DecodeDisplayUpdate(p) == false)
				out = false;
			break;
		case JOYSTICK_UPDATE:
			/* No joystick updates _from_ the master */
			if (js && TheC64->network_connection_type == MASTER)
			{
				NetworkUpdateJoystick *j = (NetworkUpdateJoystick *)p->data;
				*js = j->val;
			}
			break;
		case TEXT_MESSAGE:
		{
			static char display_buf[80];

			strncpy(display_buf, (char*)p->data, 80);
			display->display_status_string(display_buf, 4);
		} break;
		case LIST_PEERS:
		{
		} break;
		case REGISTER_DATA:
		{
			NetworkUpdateRegisterData *rd = (NetworkUpdateRegisterData *)p->data;

			DataStore::ds->registerNetworkData(rd->key, rd->metadata, rd->data,
					p->size - (sizeof(NetworkUpdateRegisterData) + sizeof(NetworkUpdate)));
		} break;
		case BANDWIDTH_PING:
		case PING:
		{
			NetworkUpdatePingAck *ping = (NetworkUpdatePingAck *)p->data;
			uint16 type = ACK;

			if (ud->type == BANDWIDTH_PING)
				type = BANDWIDTH_ACK;
			this->SendPingAck(ping->seq, type, ud->size);
		} break;
		case BANDWIDTH_ACK:
		case ACK:
			/* We won't receive this, but it also doesn't really matter */
			break;
		case DISCONNECT:
			out = false;
			break;
		default:
			break;
		}
		p = this->GetNext(p);
	}

	return out;
}

bool Network::AppendScreenshot(NetworkUpdatePeerInfo *pi)
{
	NetworkUpdateRegisterData *dsu;
	NetworkUpdate *ud;
	SDL_Surface *scr;
	void *png;
	size_t sz;
	bool out = NULL;

	scr = TheC64->TheDisplay->SurfaceFromC64Display();
	if (!scr)
		goto out_none;

	png = sdl_surface_to_png(scr, &sz);
	if (!png)
		goto out_scr;

	ud = InitNetworkUpdate(this->cur_ud, REGISTER_DATA,
			sizeof(NetworkUpdate) + sizeof(NetworkUpdateRegisterData) + sz);
	dsu = (NetworkUpdateRegisterData *)ud->data;
	dsu->key = DataStore::ds->getNextKey();
	dsu->metadata = 0;
	memcpy(dsu->data, png, sz);
	this->AddNetworkUpdate(ud);

	out = true;
	free(png);
out_scr:
	SDL_FreeSurface(scr);
out_none:

	return out;
}

bool Network::ConnectToBroker()
{
	NetworkUpdate *ud = InitNetworkUpdate(this->cur_ud, CONNECT_TO_BROKER,
			sizeof(NetworkUpdate) + sizeof(NetworkUpdatePeerInfo));
	NetworkUpdatePeerInfo *pi = (NetworkUpdatePeerInfo *)ud->data;
	bool out;

	/* Reset peer selection */
	this->peer_selected = -1;

	pi->is_master = 0; /* Will be set later */
	pi->key = ThePrefs.NetworkKey;
	pi->version = FRODO_NETWORK_PROTOCOL_VERSION;
	pi->avatar = ThePrefs.NetworkAvatar;
	pi->screenshot_key = 0;

	strcpy((char*)pi->name, ThePrefs.NetworkName);
	this->AddNetworkUpdate(ud);
	out = this->AppendScreenshot(pi);
	if (out)
		out = this->SendServerUpdate();
	this->ResetNetworkUpdate();

	return out;
}

void Network::SendPingAck(int seq, uint16 type, size_t size_to_send)
{
	NetworkUpdate *ud = InitNetworkUpdate(this->ud, type, size_to_send);
	NetworkUpdatePingAck *p = (NetworkUpdatePingAck*)ud->data;

	p->seq = seq;
	this->AddNetworkUpdate(ud);
}

network_connection_error_t Network::WaitForPeerAddress()
{
	NetworkUpdateListPeers *pi;

	this->ResetNetworkUpdate();
	if (this->ReceiveUpdate() == false)
		return AGAIN_ERROR;
	if (this->ud->type == PING)
	{
		NetworkUpdatePingAck *p = (NetworkUpdatePingAck*)ud->data;
		/* Send ack and go back to this state again */
		this->SendPingAck(p->seq, ACK, ud->size);
		this->SendServerUpdate();
		this->ResetNetworkUpdate();
		return AGAIN_ERROR;
	}
	if (this->ud->type != LIST_PEERS)
		return SERVER_GARBAGE_ERROR;

	pi = (NetworkUpdateListPeers *)this->ud->data;
	if (pi->n_peers != 1)
	{
		fprintf(stderr, "There is something wrong with the server: Got %d peers on master connect\n"
				"Contact Simon Kagstrom and ask him to correct it\n",
				pi->n_peers);
		return SERVER_GARBAGE_ERROR;
	}
	if (pi->peers[0].version != FRODO_NETWORK_PROTOCOL_VERSION)
		return VERSION_ERROR;

	return OK;
}

bool Network::SelectPeer(uint32 id)
{
	NetworkUpdate *ud = InitNetworkUpdate(this->ud, SELECT_PEER,
			sizeof(NetworkUpdate) + sizeof(NetworkUpdateSelectPeer));
	NetworkUpdateSelectPeer *p = (NetworkUpdateSelectPeer*)ud->data;
	bool out;

	p->server_id = id;
	this->AddNetworkUpdate(ud);
	out = this->SendServerUpdate();
	this->ResetNetworkUpdate();

	return out;		
}

network_connection_error_t Network::WaitForPeerList()
{
	NetworkUpdateListPeers *pi;
	struct timeval tv;

	tv.tv_sec = 1;
	tv.tv_usec = 0;

	this->ResetNetworkUpdate();
	if (this->ReceiveUpdate(&tv) == false)
		return AGAIN_ERROR;
	if (this->ud->type == PING)
	{
		NetworkUpdatePingAck *p = (NetworkUpdatePingAck*)ud->data;
		/* Send ack and go back to this state again */
		this->SendPingAck(p->seq, ACK, ud->size);
		this->SendServerUpdate();
		this->ResetNetworkUpdate();
		return AGAIN_ERROR;
	}
	if (ud->type != LIST_PEERS)
		return SERVER_GARBAGE_ERROR;

	pi = (NetworkUpdateListPeers *)this->ud->data;
	for (unsigned i = 0; i < pi->n_peers; i++)
	{
		if (pi->peers[i].version != FRODO_NETWORK_PROTOCOL_VERSION)
			return VERSION_ERROR;
	}
	if (pi->n_peers == 0)
		return NO_PEERS_ERROR;

	Gui::gui->nuv->setPeers(pi);
	Gui::gui->activate();
	Gui::gui->pushView(Gui::gui->nuv);

	return OK;
}

bool Network::SelectPeer(const char *hostname, uint16_t port, uint32_t server_id)
{
	if (!hostname)
	{
		this->peer_selected = 0;
		return true;
	}

	this->SelectPeer(server_id);
	this->InitSockaddr(&this->peer_addr, hostname, port);
	this->peer_selected = 1;

	return true;
}

network_connection_error_t Network::WaitForPeerSelection()
{
	if (this->peer_selected == 1)
		return AGAIN_ERROR;

	return OK;
}

bool Network::WaitForPeerReply()
{
	struct timeval tv;

	tv.tv_sec = 3;
	tv.tv_usec = 0;

	this->ResetNetworkUpdate();
	if (this->ReceiveUpdate(&tv) == false)
		return false;

	if (this->ud->type != CONNECT_TO_PEER)
		return false;

	return true;
}

bool Network::ConnectToPeer()
{
	NetworkUpdate *ud = InitNetworkUpdate(this->ud, CONNECT_TO_PEER,
			sizeof(NetworkUpdate));
	bool out;

	this->AddNetworkUpdate(ud);
	out = this->SendServerUpdate();
	this->ResetNetworkUpdate();

	return out;
}

network_connection_error_t Network::WaitForBandWidthReply()
{
	unsigned cnt;

	/* Wait until we've got an ack */
	for (cnt = 0; cnt < 5; cnt++) {
		struct timeval tv;

		tv.tv_sec = 3;
		tv.tv_usec = 0;
		this->ResetNetworkUpdate();

		if (this->ReceiveUpdate(&tv) == false)
			return AGAIN_ERROR;
		if (this->ud->type == BANDWIDTH_PING) {
			NetworkUpdatePingAck *ping = (NetworkUpdatePingAck *)this->ud->data;
			uint32 seq = ping->seq;
			size_t sz = this->ud->size;

			this->ResetNetworkUpdate();
			this->SendPingAck(seq, BANDWIDTH_ACK, sz);
			this->SendServerUpdate();
			continue;
		}
		/* CONNECT_TO_PEER is sent twice, so we might get it here */
		if (this->ud->type == CONNECT_TO_PEER)
			continue;
		if (this->ud->type == BANDWIDTH_ACK)
			break;
		else /* Everything else is an error */
			return SERVER_GARBAGE_ERROR;
		cnt++;
	}
	if (cnt == 5) {
		printf("Timeout. Setting default kbps (160)\n");
		this->target_kbps = 160000;
		return OK;
	}

	/* We got a bandwidth ACK */

	uint32 now = SDL_GetTicks();
	int32 ms_diff = now - this->bandwidth_ping_ms;
	size_t sz = this->ud->size;

	if (ms_diff <= 0) {
		/* Fast indeed, or maybe wrong */
		this->target_kbps = 240000;
	} else {
		int bits_per_second = ((sz * 1000) / ms_diff) * 8;

		this->target_kbps = bits_per_second;
	}

	/* But force it to be within these limits */
	if (this->target_kbps > 300000)
		this->target_kbps = 300000;
	if (this->target_kbps < 150000)
		this->target_kbps = 150000;

	printf("%d bytes in %d ms. Setting cap at %d\n", sz, ms_diff, this->target_kbps);

	return OK;
}

network_connection_error_t Network::ConnectFSM()
{
	network_connection_error_t err;

	/* See http://www.brynosaurus.com/pub/net/p2pnat/ for how this works.
	 *
	 * For the client:
	 *   1. Send connect to the broker
	 *   2. Wait for the broker to return list of peers
	 *   3. Tell the broker who to connect to
	 *   4. Select peer
	 *   	4.1 (master) Wait for broker to return peer address
	 *   	4.2 (client) Connect to peer
	 *   5. Until connected:
	 *      5.1 Send connection message to peer
	 *      5.2 Wait for reply from peer
	 *   6. Test bandwidth
	 */
	switch(this->network_connection_state)
	{
	case CONN_CONNECT_TO_BROKER:
	{
		Gui::gui->status_bar->queueMessage("Connecting to broker...");
		if (this->ConnectToBroker())
			this->network_connection_state = CONN_WAIT_FOR_PEER_LIST;
	} break;
	case CONN_WAIT_FOR_PEER_ADDRESS:
		Gui::gui->status_bar->queueMessage("Waiting for connection...");
		err = this->WaitForPeerAddress();
		if (err == OK)
			this->network_connection_state = CONN_CONNECT_TO_PEER;
		else
			return err;
		break;
	case CONN_WAIT_FOR_PEER_LIST:
		Gui::gui->status_bar->queueMessage("Waiting for peer list...");
		/* Also tells the broker that we want to connect */
		err = this->WaitForPeerList();
		if (err == NO_PEERS_ERROR)
		{
                        this->network_connection_state = CONN_WAIT_FOR_PEER_ADDRESS;
                        this->is_master = true;
		}
		else
			this->network_connection_state = CONN_WAIT_FOR_PEER_SELECT;
		break;
	case CONN_WAIT_FOR_PEER_SELECT:
		err = this->WaitForPeerSelection();
		if (err == OK) {
			this->network_connection_state = CONN_CONNECT_TO_PEER;
			this->is_master = false;
		}
		else if (err == NO_PEERS_ERROR) {
			this->network_connection_state = CONN_WAIT_FOR_PEER_ADDRESS;
			this->is_master = true;
		}
		else
			return err;
		break;
	case CONN_CONNECT_TO_PEER:
		Gui::gui->status_bar->queueMessage("Connecting to peer...");
		if (this->ConnectToPeer() == false)
			return AGAIN_ERROR;
		/* Allow some transit time */
		sleep(1);
		if (this->ConnectToPeer() == false)
			return AGAIN_ERROR;
		this->network_connection_state = CONN_WAIT_FOR_PEER_REPLY;
		break;
	case CONN_WAIT_FOR_PEER_REPLY:
		/* Connect again in case the first sent was dropped on
		 * its way to the peer */
		if (this->WaitForPeerReply() == true)
			this->network_connection_state = CONN_BANDWIDTH_PING;
		else
			return AGAIN_ERROR;
		break;
	case CONN_BANDWIDTH_PING:
		this->ResetNetworkUpdate();
		this->SendPingAck(this->is_master, BANDWIDTH_PING, 1024);
		this->SendServerUpdate();
		this->bandwidth_ping_ms = SDL_GetTicks();
		this->ResetNetworkUpdate();
		this->network_connection_state = CONN_BANDWIDTH_REPLY;
		break;
	case CONN_BANDWIDTH_REPLY:
	{
		network_connection_error_t err = this->WaitForBandWidthReply();
		if (err == OK) {
			this->network_connection_state = CONN_CONNECTED;
			return AGAIN_ERROR;
		}
		return err;
	} break;
	case CONN_CONNECTED:
		Gui::gui->status_bar->queueMessage("Connected!");
		/* The lowest number is the default master */
	default:
		return OK;
	}

	return AGAIN_ERROR;
}


void Network::Disconnect()
{
	NetworkUpdate *disconnect = InitNetworkUpdate(this->cur_ud, DISCONNECT,
			sizeof(NetworkUpdate));

	/* Add a stop at the end of the update */
	this->AddNetworkUpdate(disconnect);
	this->SendServerUpdate();
	if (this->network_connection_state)
		this->SendPeerUpdate();
}

bool Network::networking_started = false;

#if defined(GEKKO)
#include "NetworkWii.h"
#else
#include "NetworkUnix.h"
#endif
