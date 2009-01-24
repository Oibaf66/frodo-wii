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

#define RAW_SIZE ( (SQUARE_W * SQUARE_H) / 2 )

Network::Network()
{
	const size_t size = NETWORK_UPDATE_SIZE;

	/* "big enough" static buffer */
	this->ud = (NetworkUpdate*)malloc( size );
	this->ResetNetworkUpdate();
	this->bytes_sent = 0;
}

Network::~Network()
{
	free(this->ud);
}

size_t Network::EncodeDisplayRaw(struct NetworkDisplayUpdate *dst, Uint8 *screen,
		int x_start, int y_start)
{
	const int raw_w = SQUARE_W / 2;
	dst->type = DISPLAY_UPDATE_RAW;

	for (int y = y_start; y < y_start + SQUARE_H; y++)
	{
		for (int x = x_start; x < x_start + SQUARE_W; x += 2)
		{
			Uint8 a = screen[ y * DISPLAY_X + x ];
			Uint8 b = screen[ y * DISPLAY_X + (x + 1) ];

			dst->data[ (y - y_start) * raw_w + (x - x_start) / 2 ] =
				(a << 4) | b;
		}
	}

	return RAW_SIZE; 
}

size_t Network::EncodeDisplayRLE(struct NetworkDisplayUpdate *dst, Uint8 *screen,
		int x_start, int y_start)
{
	size_t out = 0;
	size_t len = 0;
	Uint8 color = screen[ y_start * DISPLAY_X + x_start ];

	dst->type = DISPLAY_UPDATE_RLE;

	for (int y = y_start; y < y_start + SQUARE_H; y++)
	{
		for (int x = x_start; x < x_start + SQUARE_W; x++)
		{
			if (color != screen[ y * DISPLAY_X + x ] ||
					len >= 255)
			{
				dst->data[out] = len;
				dst->data[out + 1] = color;
				out += 2;

				len = 0;
				color = screen[ y * DISPLAY_X + x];
			}
			len++;
		}
	}
	if (len != 0)
	{
		dst->data[out] = len;
		dst->data[out + 1] = color;

		out += 2;
	}

	return out;
}

size_t Network::EncodeSoundRLE(struct NetworkSoundUpdate *dst,
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

	return out;
}

size_t Network::EncodeSoundRaw(struct NetworkSoundUpdate *dst,
		Uint8 *buffer, size_t len)
{
	dst->type = SOUND_UPDATE_RAW;
	memcpy(dst->data, buffer, len);

	return len;
}


bool Network::DecodeDisplayRLE(Uint8 *screen, struct NetworkDisplayUpdate *src,
		int x_start, int y_start)
{
	int p = 0;
	int x = x_start;
	int y = y_start;
	int sz = src->size - sizeof(NetworkDisplayUpdate);

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

bool Network::DecodeDisplayRaw(Uint8 *screen, struct NetworkDisplayUpdate *src,
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

/* Public methods */
size_t Network::EncodeDisplaySquare(struct NetworkDisplayUpdate *dst,
		Uint8 *screen, int square)
{
	const int square_x = SQUARE_TO_X(square);
	const int square_y = SQUARE_TO_Y(square);
	size_t out;

	dst->square = square;
	/* Try encoding as RLE, but if it's too large, go for RAW */
	out = this->EncodeDisplayRLE(dst, screen, square_x, square_y);
	if (out > RAW_SIZE)
		out = this->EncodeDisplayRaw(dst, screen, square_x, square_y);
	dst->size = out + sizeof(struct NetworkDisplayUpdate);

	return dst->size;
}

bool Network::CompareSquare(Uint8 *a, Uint8 *b)
{
	for (int y = 0; y < SQUARE_H; y++)
	{
		for (int x = 0; x < SQUARE_W; x++)
		{
			Uint8 va = a[ y * DISPLAY_X + x ];
			Uint8 vb = b[ y * DISPLAY_X + x ];

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
		Uint8 *p_remote= &remote[ SQUARE_TO_Y(sq) * DISPLAY_X + SQUARE_TO_X(sq) ]; 

		if (this->CompareSquare(p_master, p_remote) == false)
		{
			NetworkDisplayUpdate *dst = (NetworkDisplayUpdate *)this->cur_ud;

			/* Updated, encode this */
			this->EncodeDisplaySquare(dst, master, sq);
			this->AddNetworkUpdate((NetworkUpdate*)dst);
		}
	}

	/* Everything encoded, store in remote */
	memcpy(remote, master, DISPLAY_X * DISPLAY_Y);
}


bool Network::DecodeDisplayUpdate(Uint8 *screen,
		struct NetworkDisplayUpdate *src)
{
	int square = src->square;
	const int square_x = SQUARE_TO_X(square);
	const int square_y = SQUARE_TO_Y(square);

	if (src->type == DISPLAY_UPDATE_RAW)
		return this->DecodeDisplayRaw(screen, src, square_x, square_y);
	if (src->type == DISPLAY_UPDATE_RLE)
		return this->DecodeDisplayRLE(screen, src, square_x, square_y);

	/* Error */
	return false;
}

size_t Network::EncodeSoundBuffer(struct NetworkSoundUpdate *dst, Uint8 *buf, size_t len)
{
	size_t out;

	dst->size = 0;
	/* Try encoding as RLE, but if it's too large, go for RAW */
	out = this->EncodeSoundRLE(dst, buf, len);
	if (out > len)
		out = this->EncodeSoundRaw(dst, buf, len);
	dst->size = out + sizeof(struct NetworkSoundUpdate); 

	return dst->size;
}

void Network::EncodeJoystickUpdate(struct NetworkJoystickUpdate *dst, Uint8 which, Uint8 v)
{
	dst->type = JOYSTICK_UPDATE;
	dst->which = which;
	dst->data = v;
}


size_t Network::DecodeSoundUpdate(struct NetworkSoundUpdate *src, char *buf)
{
	size_t out;

	if (src->type == SOUND_UPDATE_RAW)
	{
		out = src->size - sizeof(struct NetworkSoundUpdate);
		memcpy(buf, src->data, out);
	}
	else
		out = 0; 

	return out;
}

void Network::ResetNetworkUpdate(void)
{
	memset(this->ud, 0, NETWORK_UPDATE_SIZE);

	this->ud->type = HEADER;
	this->ud->size = sizeof(NetworkUpdate);
	this->cur_ud = (Uint8*)(this->ud->data);
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

void Network::AddNetworkUpdate(NetworkUpdate *update)
{
	size_t sz = update->size;

	this->cur_ud += sz;
	this->ud->size += sz;
}


void Network::MarshalData(NetworkUpdate *ud)
{
	Uint8 *p = ud->data;
	int len = 0;
	int sz = ud->size;

	ud->size = htons(ud->size);
	while (len < sz)
	{
		p = p + len;

		switch (p[0])
		{
		case DISPLAY_UPDATE_RAW:
		case DISPLAY_UPDATE_RLE:
		case SOUND_UPDATE_RAW: /* Not really true, but looks the same */
		case SOUND_UPDATE_RLE:
		{
			NetworkDisplayUpdate *tmp = (NetworkDisplayUpdate *)p;

			len += tmp->size;
			tmp->size = htons(tmp->size);
		} break;
		case JOYSTICK_UPDATE:
			len = sizeof(NetworkJoystickUpdate);
			break;
		default:
			/* Unknown data... */
			return;
		}
	}
}

void Network::DeMarshalData(NetworkUpdate *ud)
{
	Uint8 *p = ud->data;
	int len = 0;

	ud->size = ntohs(ud->size);
	while (len < ud->size)
	{
		p = p + len;

		switch (p[0])
		{
		case DISPLAY_UPDATE_RAW:
		case DISPLAY_UPDATE_RLE:
		case SOUND_UPDATE_RAW: /* Not really true, but looks the same */
		case SOUND_UPDATE_RLE:
		{
			NetworkDisplayUpdate *tmp = (NetworkDisplayUpdate *)p;

			tmp->size = ntohs(tmp->size);
			len += tmp->size;
		} break;
		case JOYSTICK_UPDATE:
			len = sizeof(NetworkJoystickUpdate);
			break;
		default:
			/* Unknown data... */
			return;
		}
	}
}

bool Network::DecodeUpdate(uint8 *screen)
{
	unsigned int cookie;
	NetworkUpdate *p;
	int i = 0;

	for (p = this->IterateFirst(this->ud, &cookie); p;
		p = this->IterateNext(this->ud, &cookie))
	{
		switch(p->type)
		{
		case DISPLAY_UPDATE_RAW:
		case DISPLAY_UPDATE_RLE:
			this->DecodeDisplayUpdate(screen, (NetworkDisplayUpdate*)p);
			break;
		default:
			break;
		}
		i++;
	}
}

NetworkUpdate *Network::IterateFirst(NetworkUpdate *p, unsigned int *cookie)
{
	Uint8 *p8 = (Uint8*)p;
	NetworkUpdate *cur = (NetworkUpdate *)p->data;

	*cookie = (unsigned int)(p->data - p8);
	if ( *cookie >= p->size )
		return NULL;
	*cookie = *cookie + cur->size;

	return cur; 
}

NetworkUpdate *Network::IterateNext(NetworkUpdate *p, unsigned int *cookie)
{
	NetworkUpdate *cur = (NetworkUpdate *)(((Uint8*)p) + *cookie);

	/* End of iteration */
	if ( *cookie >= p->size || cur->size == 0)
		return NULL;

	*cookie = *cookie + cur->size;

	return cur;
}

void NetworkServer::AddClient(int sock)
{
	NetworkClient *cli = new NetworkClient(sock);

	this->clients[this->n_clients] = cli;
	this->n_clients++;
}


NetworkClient::NetworkClient(int sock)
{
	this->sock = sock;

	this->screen = (Uint8 *)malloc(DISPLAY_X * DISPLAY_Y);
	assert(this->screen);

	/* Assume black screen */
	memset(this->screen, 0, DISPLAY_X * DISPLAY_Y);
}

NetworkClient::~NetworkClient()
{
	free(this->screen);
}

#include "NetworkUnix.h"
