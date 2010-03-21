/*
 *  SID_SDL.h - 6581 emulation, SDL specific stuff
 *
 *  Frodo (C) 1994-1997,2002-2005 Christian Bauer
 *
 *  SDL stuff by Simon Kagstrom
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

#include <unistd.h>
#include <fcntl.h>
#include <SDL.h>

#include "VIC.h"

static SDL_AudioSpec spec;

#define FRODO_SNDBUF 512
#define SOUNDBUFSIZE 1536
#define N_BUFS 8

static int16 soundbuffer[N_BUFS][FRODO_SNDBUF];
static int head, tail;

static void fill_audio(void *udata, Uint8 *stream, int len)
{
	const int sz = FRODO_SNDBUF * 2;
	int off = 0;

	memset(stream, 0, len);
	while (tail != head)
	{
		//printf("Copying %d bytes at off %d, tail %d, head %d\n", len, off, tail, head);
		memcpy(stream + off, soundbuffer[tail], sz);
		off += sz;
		if (off >= len)
			break;
		tail = (tail + 1) % N_BUFS;
	}
}

/*
 *  Initialization
 */

void DigitalRenderer::init_sound(void)
{
	this->sndbufsize = FRODO_SNDBUF;

	/* Set the audio format */
	spec.freq = 32000;
	spec.format = AUDIO_S16SYS;
	spec.channels = 1;    /* 1 = mono, 2 = stereo */
	spec.samples = SOUNDBUFSIZE;
	spec.callback = fill_audio;
	spec.userdata = (void*)this;

	ready = false;
	if ( SDL_OpenAudio(&spec, NULL) < 0 ) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		return ;
	}

	this->sound_buffer = new int16[this->sndbufsize];
	memset(this->sound_buffer, 0, sizeof(int16) * this->sndbufsize);
	ready = true;
	SDL_PauseAudio(0);
}


/*
 *  Destructor
 */

DigitalRenderer::~DigitalRenderer()
{
	SDL_CloseAudio();
}


/*
 *  Pause sound output
 */

void DigitalRenderer::Pause(void)
{
	SDL_PauseAudio(1);
}


/*
 * Resume sound output
 */

void DigitalRenderer::Resume(void)
{
	SDL_PauseAudio(0);
}


/*
 * Fill buffer, sample volume (for sampled voice)
 */

void DigitalRenderer::PushVolume(uint8 vol)
{
	static int divisor = 0;
	static int to_output = 0;

	sample_buf[sample_in_ptr] = volume;
	sample_in_ptr = (sample_in_ptr + 1) % SAMPLE_BUF_SIZE;

	/*
	 * Now see how many samples have to be added for this line
	 */
	divisor += SAMPLE_FREQ;
	while (divisor >= 0)
	{
		divisor -= TOTAL_RASTERS*SCREEN_FREQ;
		to_output++;
	}

	if (to_output >= sndbufsize) {
		int datalen = sndbufsize;
		to_output -= datalen;

		SDL_LockAudio();
		calc_buffer(soundbuffer[head], datalen * 2);
		head = (head + 1) % N_BUFS;
		if (head == tail) {
			tail = (head + 1) % N_BUFS;
		}
		SDL_UnlockAudio();
	}
}


void DigitalRenderer::EmulateLine(void)
{
	if (!ready)
		return;
	this->PushVolume(volume);
}
