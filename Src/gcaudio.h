/****************************************************************************
 * From the FCE Ultra 0.98.12
 * Nintendo Wii/Gamecube Port
 *
 * Tantric September 2008
 * eke-eke October 2008
 * Simon Kagstrom Jan 2009
 *
 * gcaudio.h
 *
 * Audio driver
 ****************************************************************************/

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

void InitialiseAudio();
void StopAudio();
void ResetAudio();
void PlaySound( int16_t *Buffer, int samples );

#if defined(__cplusplus)
};
#endif
