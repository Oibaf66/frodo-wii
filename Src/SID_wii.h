/*
 *  SID_linux.i - 6581 emulation, Linux specific stuff
 *
 *  Frodo (C) 1994-1997,2002 Christian Bauer
 *  Wii sound stuff by Simon Kagstrom
 */

#include "gcaudio.h"

#include "VIC.h"
#include "Network.h"

/*
 *  Initialization
 */

void DigitalRenderer::init_sound(void)
{
	this->sndbufsize = 512;

	ready = false;
	InitialiseAudio();
	ResetAudio();

	this->sound_buffer = new int16[this->sndbufsize];
	memset(this->sound_buffer, 0, sizeof(int16) * this->sndbufsize);
	ready = true;
}


/*
 *  Destructor
 */

DigitalRenderer::~DigitalRenderer()
{
	StopAudio();
}


/*
 *  Pause sound output
 */

void DigitalRenderer::Pause(void)
{
	StopAudio();
}


/*
 * Resume sound output
 */

void DigitalRenderer::Resume(void)
{
	/* Done by PlayAudio() */
}


/*
 * Fill buffer, sample volume (for sampled voice)
 */

void DigitalRenderer::PushVolume(uint8 volume)
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
		divisor -= TOTAL_RASTERS*SCREEN_FREQ, to_output++;

	/*
	 * Calculate the sound data only when we have enough to fill
	 * the buffer entirely.
	 */
	if (to_output >= sndbufsize) {
		int datalen = sndbufsize;
		to_output -= datalen;
		calc_buffer(sound_buffer, datalen * 2);

		PlaySound(sound_buffer, datalen);
	}
}

void DigitalRenderer::EmulateLine(void)
{
	if (!ready || TheC64->IsPaused())
		return;
	this->PushVolume(volume);
}
