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

Network::Network()
{
	const size_t size = NETWORK_UPDATE_SIZE;

	/* "big enough" buffer */
	this->ud = (NetworkUpdate*)malloc( size );
	this->tmp_ud = (NetworkUpdate*)malloc( size );
	assert(this->ud && this->tmp_ud);

	this->ResetNetworkUpdate();
	this->traffic = 0;
	this->last_traffic = 0;
	this->target_kbps = 120000; /* kilobit per seconds */
	this->kbps = 0;

	this->raw_buf = (Uint8*)malloc(RAW_SIZE);
	this->rle_buf = (Uint8*)malloc(RLE_SIZE);
	this->diff_buf = (Uint8*)malloc(DIFF_SIZE);
	assert(this->raw_buf && this->rle_buf && this->diff_buf);

	this->square_updated = (Uint32*)malloc( N_SQUARES_W * N_SQUARES_H * sizeof(Uint32));
	assert(this->square_updated);
	memset(this->square_updated, 0, N_SQUARES_W * N_SQUARES_H * sizeof(Uint32));
}

Network::~Network()
{
	free(this->ud);
	free(this->tmp_ud);
	free(this->square_updated);
	free(this->raw_buf);
	free(this->rle_buf);
	free(this->diff_buf);
}

void Network::Tick(int ms)
{
	int last_kbps = ((this->traffic - this->last_traffic) * 8) * (1000 / ms);

	/* 1/3 of the new value, 2/3 of the old */
	this->kbps = 2 * (this->kbps / 3) + (last_kbps / 3);
	this->last_traffic = this->traffic;
}

size_t Network::EncodeSoundRLE(struct NetworkUpdate *dst,
		Uint8 *buffer, size_t buf_len)
{
	size_t out = 0;
	size_t len = 0;
	Uint8 volume = buffer[0];

	dst->type = SOUND_UPDATE_RLE;

	for (unsigned int i = 0; i < buf_len; i++)
	{
		if (volume != buffer[i] ||
				len >= 255)
		{
			dst->data[out] = len;
			dst->data[out + 1] = volume;
			out += 2;

			len = 0;
			volume = buffer[i];
		}
		len++;
	}
	if (len != 0)
	{
		dst->data[out] = len;
		dst->data[out + 1] = volume;

		out += 2;
	}

	return out;
}

size_t Network::EncodeSoundRaw(struct NetworkUpdate *dst,
		Uint8 *buffer, size_t len)
{
	dst->type = SOUND_UPDATE_RAW;
	memcpy(dst->data, buffer, len);

	return len;
}

bool Network::DecodeDisplayDiff(Uint8 *screen, struct NetworkUpdate *src,
		int x_start, int y_start)
{
	int p = 0;
	int x = x_start;
	int y = y_start;
	int sz = src->size - sizeof(NetworkUpdate);

	/* Something is wrong if this is true... */
	if (sz % 2 != 0)
		return false;

	while (p < sz)
	{
		Uint8 len = src->data[p];
		Uint8 color = src->data[p+1];
		int x_diff = (x - x_start + len) % SQUARE_W;
		int y_diff = (x - x_start + len) / SQUARE_W;

		x = x_start + x_diff;
		y = y + y_diff;
		screen[y * DISPLAY_X + x] = color;
		p += 2;
	}

	return true;
}

bool Network::DecodeDisplayRLE(Uint8 *screen, struct NetworkUpdate *src,
		int x_start, int y_start)
{
	int p = 0;
	int x = x_start;
	int y = y_start;
	int sz = src->size - sizeof(NetworkUpdate);

	/* Something is wrong if this is true... */
	if (sz % 2 != 0)
		return false;

	while (p < sz)
	{
		Uint8 len = src->data[p];
		Uint8 color = src->data[p+1];

		while (len > 0)
		{
			screen[y * DISPLAY_X + x] = color;
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

bool Network::DecodeDisplayRaw(Uint8 *screen, struct NetworkUpdate *src,
		int x_start, int y_start)
{
	const int raw_w = SQUARE_W / 2;

	for (int y = y_start; y < y_start + SQUARE_H; y++)
	{
		for (int x = x_start; x < x_start + SQUARE_W; x += 2)
		{
			Uint8 v = src->data[(y - y_start) * raw_w + (x - x_start) / 2];
			Uint8 a = v >> 4;
			Uint8 b = v & 0xf;

			screen[ y * DISPLAY_X + x ] = a;
			screen[ y * DISPLAY_X + x + 1 ] = b;
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

size_t Network::EncodeDisplay(Uint8 *master, Uint8 *remote)
{
	for ( int sq = 0; sq < N_SQUARES_H * N_SQUARES_W; sq++ )
	{
		Uint8 *p_master = &master[ SQUARE_TO_Y(sq) * DISPLAY_X + SQUARE_TO_X(sq) ]; 
		Uint8 *p_remote = &remote[ SQUARE_TO_Y(sq) * DISPLAY_X + SQUARE_TO_X(sq) ]; 

		if (this->CompareSquare(p_master, p_remote) == false)
		{
			NetworkUpdate *dst = (NetworkUpdate *)this->cur_ud;

			/* Updated, encode this */
			this->EncodeDisplaySquare(dst, master, remote, sq);
			this->AddNetworkUpdate(dst);
		}
		else
			this->square_updated[sq] = 0;
	}
	memcpy(remote, master, DISPLAY_X * DISPLAY_Y);
}


size_t Network::EncodeDisplaySquare(struct NetworkUpdate *dst,
		Uint8 *screen, Uint8 *remote, int square)
{
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
	if (diff_sz < rle_sz && diff_sz < RAW_SIZE)
	{
		memcpy(dst->data, this->diff_buf, diff_sz);
		type = DISPLAY_UPDATE_DIFF;
		out = diff_sz;
	}
	else if (rle_sz < RAW_SIZE)
	{
		memcpy(dst->data, this->rle_buf, rle_sz);
		type = DISPLAY_UPDATE_RLE;
		out = rle_sz;
	}		
	else
		memcpy(dst->data, this->raw_buf, RAW_SIZE);

	/* Setup the structure */
	dst->type = type;
	dst->u.display.square = square;
	dst->size = out + sizeof(struct NetworkUpdate);
	this->square_updated[square] = out | (type << 16);

	return dst->size;
}

bool Network::DecodeDisplayUpdate(Uint8 *screen,
		struct NetworkUpdate *src)
{
	int square = src->u.display.square;
	const int square_x = SQUARE_TO_X(square);
	const int square_y = SQUARE_TO_Y(square);

	if (src->type == DISPLAY_UPDATE_DIFF)
		return this->DecodeDisplayDiff(screen, src, square_x, square_y);
	else if (src->type == DISPLAY_UPDATE_RAW)
		return this->DecodeDisplayRaw(screen, src, square_x, square_y);
	else if (src->type == DISPLAY_UPDATE_RLE)
		return this->DecodeDisplayRLE(screen, src, square_x, square_y);

	/* Error */
	return false;
}

size_t Network::EncodeSoundBuffer(struct NetworkUpdate *dst, Uint8 *buf, size_t len)
{
	size_t out;

	dst->size = 0;
	/* Try encoding as RLE, but if it's too large, go for RAW */
	out = this->EncodeSoundRLE(dst, buf, len);
	if (out > len)
		out = this->EncodeSoundRaw(dst, buf, len);
	dst->size = out + sizeof(struct NetworkUpdate); 

	return dst->size;
}

void Network::EncodeJoystickUpdate(Uint8 v)
{
	struct NetworkUpdate *dst = this->cur_ud;

	dst->type = JOYSTICK_UPDATE;
	dst->u.joystick.val = v;
	dst->size = sizeof(NetworkUpdate);

	this->AddNetworkUpdate(dst);
}


size_t Network::DecodeSoundUpdate(struct NetworkUpdate *src, char *buf)
{
	size_t out;

	if (src->type == SOUND_UPDATE_RAW)
	{
		out = src->size - sizeof(struct NetworkUpdate);
		memcpy(buf, src->data, out);
	}
	else
		out = 0; 

	return out;
}

void Network::ResetNetworkUpdate(void)
{
	memset(this->ud, 0, NETWORK_UPDATE_SIZE);
	memset(this->tmp_ud, 0, NETWORK_UPDATE_SIZE);

	this->ud->type = STOP;
	this->ud->size = sizeof(NetworkUpdate);
	this->cur_ud = this->ud;
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

bool Network::ReceiveUpdate(int sock)
{
	struct timeval tv;

	memset(&tv, 0, sizeof(tv));
	return this->ReceiveUpdate(this->ud, sock, &tv);
}


bool Network::ReceiveUpdateBlock(int sock)
{
	return this->ReceiveUpdate(this->ud, sock, NULL);
}

bool Network::ReceiveUpdate(NetworkUpdate *dst, int sock, struct timeval *tv)
{
	Uint8 *pp = (Uint8*)dst;
	NetworkUpdate *p;

	if (this->Select(sock, tv) == false)
		return false;
	/* Receive until the stop */
	do
	{
		p = (NetworkUpdate*)pp;

		/* Receive the header */
		if (this->ReceiveData((void*)p, sock, sizeof(NetworkUpdate)) == false)
			return false;

		pp = pp + sizeof(NetworkUpdate);

		/* And the rest of the update */
		size_t sz = ntohs(p->size);
		if (sz > sizeof(NetworkUpdate))
		{
			size_t sz_diff = sz - sizeof(NetworkUpdate);

			if (this->ReceiveData((void*)pp, sock, sz_diff) == false)
				return false;
			pp = pp + sz_diff; 
		}
		if (this->DeMarshalData(p) == false)
			return false;
	} while ( !(p->type == STOP && p->u.stop.val == STOP) );

	return true;
}

bool Network::SendUpdate(int sock)
{
	NetworkUpdate *src = this->ud;
	NetworkUpdate *stop = this->cur_ud;
	size_t sz;

	/* Add a stop at the end of the update */
	stop->type = STOP;
	stop->u.stop.val = STOP;
	stop->size = sizeof(NetworkUpdate);
	this->AddNetworkUpdate(stop);

	if (this->MarshalAllData(src) == false)
		return false;

	sz = this->GetNetworkUpdateSize();
	if (sz <= 0)
		return false;
	if (this->SendData((void*)src, sock, sz) == false)
		return false;
	this->traffic += sz;

	return true;
}


void Network::AddNetworkUpdate(NetworkUpdate *update)
{
	size_t sz = update->size;
	Uint8 *next = (Uint8*)this->cur_ud + update->size;

	this->cur_ud = (NetworkUpdate*)next;
}


bool Network::MarshalData(NetworkUpdate *p)
{
	p->size = htons(p->size);
	switch (p->type)
	{
	case DISPLAY_UPDATE_RAW:
	case DISPLAY_UPDATE_RLE:
	case DISPLAY_UPDATE_DIFF:
	case SOUND_UPDATE_RAW:
	case SOUND_UPDATE_RLE:
	case JOYSTICK_UPDATE:
	case DISCONNECT:
	case STOP:
		break;
	default:
		/* Unknown data... */
		fprintf(stderr, "Got unknown data %d while marshalling. Something is wrong\n",
				p->type);
		return false;
	}

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
	p->size = ntohs(p->size);

	switch (p->type)
	{
	case DISPLAY_UPDATE_RAW:
	case DISPLAY_UPDATE_RLE:
	case DISPLAY_UPDATE_DIFF:
	case SOUND_UPDATE_RAW:
	case SOUND_UPDATE_RLE:
	case JOYSTICK_UPDATE:
	case DISCONNECT:
	case STOP:
		/* Nothing to do, just bytes */
		break;
	default:
		/* Unknown data... */
		return false;
	}

	return true;
}

bool Network::DecodeUpdate(uint8 *screen, uint8 *js, bool server)
{
	NetworkUpdate *p = this->ud;
	bool out = true;

	while (p->type != STOP)
	{
		switch(p->type)
		{
		case DISPLAY_UPDATE_RAW:
		case DISPLAY_UPDATE_RLE:
		case DISPLAY_UPDATE_DIFF:
			if (screen == NULL)
				break;
			if (this->DecodeDisplayUpdate(screen, p) == false)
				out = false;
			break;
		case JOYSTICK_UPDATE:
			if (js)
				*js = p->u.joystick.val;
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

void NetworkServer::AddClient(int sock)
{
	NetworkClient *cli = new NetworkClient(sock);

	this->clients[this->n_clients] = cli;
	this->n_clients++;
}

void NetworkServer::RemoveClient(NetworkClient *client)
{
	for (int i = 0; i < this->n_clients; i++)
	{
		if (this->clients[i] == client)
		{
			if (i < this->n_clients - 1)
			{
				/* Swap with last */
				this->clients[i] = this->clients[this->n_clients - 1];
			}
			delete client;
			this->n_clients--;
			return;
		}
	}
	/* Not found */
}

void NetworkClient::Init()
{
	this->screen = (Uint8 *)malloc(DISPLAY_X * DISPLAY_Y);
	assert(this->screen);

	/* Assume black screen */
	memset(this->screen, 0, DISPLAY_X * DISPLAY_Y);
}


NetworkClient::NetworkClient(int sock)
{
	this->Init();
	this->sock = sock;
}

NetworkClient::~NetworkClient()
{
	free(this->screen);
	this->CloseSocket(this->sock);
}

void NetworkClient::Disconnect()
{
	NetworkUpdate *disconnect= this->cur_ud;
	size_t sz;

	/* Add a stop at the end of the update */
	disconnect->type = DISCONNECT;
	disconnect->size = sizeof(NetworkUpdate);
	this->AddNetworkUpdate(disconnect);
	this->SendUpdate();
}

#include "NetworkUnix.h"
