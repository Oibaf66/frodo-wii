/*
 *  SID_linux.i - 6581 emulation, Linux specific stuff
 *
 *  Frodo (C) 1994-1997,2002 Christian Bauer
 *  Linux sound stuff by Bernd Schmidt
 */

#include <unistd.h>
#include <fcntl.h>
#include <SDL.h>
#include <SDL_mixer.h>

#include "VIC.h"

static int divisor = 0;
static int to_output = 0;

void DigitalRenderer::fill_audio(Uint8 *stream, int len)
{
 	int cnt = 0;
	int bytes_to_write = to_output * sizeof(Uint16);
	int buf_size = this->sndbufsize * sizeof(Uint16);

	/* Wii is stereo-only, so divide the buffer by two */
	memset(stream, 0, len);
	len = len / 2;
	if (to_output <= 0)
	        return;

	while (cnt < bytes_to_write && cnt < len)
	{
	        int datalen = buf_size;
		static Uint16 real_buf[4096];
	        int i;

		if (datalen > bytes_to_write)
	                datalen = bytes_to_write;

	        if (datalen > len - cnt)
	                datalen = len - cnt;

                calc_buffer(sound_buffer, datalen);
                /* ... and calculate a real stereo buffer */
                for (i = 0; i < datalen; i++)
                  {
                    real_buf[i*2] = sound_buffer[i];
                    real_buf[i*2+1] = sound_buffer[i];
                  }
                /* and output it */
		memcpy(stream + cnt*2, (Uint8*)real_buf, datalen * 2);
		cnt += datalen;
        }
        if (to_output - cnt / 2 <= 0)
                to_output = 0;
	else
                to_output -= cnt / 2;
}

void DigitalRenderer::fill_audio_helper(void *udata, Uint8 *stream, int len)
{
	DigitalRenderer *p_this = (DigitalRenderer *)udata;

	p_this->fill_audio(stream, len);
}

/*
 *  Initialization
 */

void DigitalRenderer::init_sound(void)
{
	this->sndbufsize = 512;

	/* Set the audio format */
	this->spec.freq = 44100;
	this->spec.format = AUDIO_S16SYS;
	this->spec.channels = 2;    /* 1 = mono, 2 = stereo */
	this->spec.samples = 512;
	this->spec.callback = this->fill_audio_helper;
	this->spec.userdata = (void*)this;

	ready = false;
	if ( SDL_OpenAudio(&this->spec, NULL) < 0 ) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		return ;
	}

	this->sound_buffer = new int16[this->sndbufsize];
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

void DigitalRenderer::EmulateLine(void)
{
	if (!ready)
		return;

	sample_buf[sample_in_ptr] = volume;
	sample_in_ptr = (sample_in_ptr + 1) % SAMPLE_BUF_SIZE;

	/*
	 * Now see how many samples have to be added for this line
	 */
	divisor += SAMPLE_FREQ;
	while (divisor >= 0)
		divisor -= TOTAL_RASTERS*SCREEN_FREQ, to_output++;
}
