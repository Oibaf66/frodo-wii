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

#define N_SQUARES_W 6
#define N_SQUARES_H 4

#define SQUARE_W (DISPLAY_X / N_SQUARES_W)
#define SQUARE_H (DISPLAY_Y / N_SQUARES_H)

#define SQUARE_TO_X(square) ( ((square) % N_SQUARES_W) * SQUARE_W )
#define SQUARE_TO_Y(square) ( ((square) / N_SQUARES_W) * SQUARE_H )

#define RAW_SIZE ( (SQUARE_W * SQUARE_H) / 2 )

size_t Network::EncodeDisplayRaw(struct NetworkDisplayUpdate *dst, Uint8 *screen,
		int x_start, int y_start)
{
	const int raw_w = SQUARE_W / 2;
	dst->type = DISPLAY_UPDATE_RAW;

	for (int y = y_start; y < y_start + SQUARE_W; y++)
	{
		for (int x = x_start; x < x_start + SQUARE_H; x += 2)
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

	for (int y = y_start; y < y_start + SQUARE_W; y++)
	{
		for (int x = x_start; x < x_start + SQUARE_H; x++)
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

	/* Something is wrong if this is true... */
	if (src->size % 2 != 0)
		return false;

	while (p < src->size)
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

	for (int y = y_start; y < y_start + SQUARE_W; y++)
	{
		for (int x = x_start; x < x_start + SQUARE_H; x += 2)
		{
			Uint8 x = src->data[(y - y_start) * raw_w + (x - x_start) / 2];
			Uint8 a = x >> 4;
			Uint8 b = x & 0xf;

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

NetworkUpdate *Network::GetNetworkUpdate(void)
{
	static NetworkUpdate *out;
	const size_t size = NETWORK_UPDATE_SIZE;

	if (!out)
	{
		/* "big enough" static buffer */
		out = (NetworkUpdate*)malloc( size );
	}
	memset(out, 0, size);

	out->type = HEADER;
	out->size = 0;

	return out;
}


bool Network::ReceiveUpdate(NetworkUpdate *dst, int sock)
{
	struct timeval tv;

	memset(&tv, 0, sizeof(tv));
	return this->ReceiveUpdate(dst, sock, &tv);
}


bool Network::ReceiveUpdateBlock(NetworkUpdate *dst, int sock)
{
	return this->ReceiveUpdate(dst, sock, NULL);
}

void Network::MarshalData(NetworkUpdate *ud)
{
	Uint8 *p = ud->data;
	int len = 0;
	int sz = ud->size;

	ud->size = htons(ud->size);
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

			len = tmp->size;
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

			len = tmp->size;
			tmp->size = ntohl(tmp->size);
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

NetworkUpdate *Network::IterateFirst(NetworkUpdate *p, unsigned int *cookie)
{
	Uint8 *p8 = (Uint8*)p;
	NetworkUpdate *cur = (NetworkUpdate *)p->data;

	*cookie = (unsigned int)(p->data - p8);
	if ( *cookie >= p->size )
		return NULL;
	*cookie = *cookie + cur->size;

	return (NetworkUpdate *)p->data; 
}

NetworkUpdate *Network::IterateNext(NetworkUpdate *p, unsigned int *cookie)
{
	NetworkUpdate *cur = (NetworkUpdate *)(p->data + *cookie);

	/* End of iteration */
	if ( *cookie >= p->size )
		return NULL;

	*cookie = *cookie + cur->size;

	return cur;
}


#include "NetworkUnix.h"
