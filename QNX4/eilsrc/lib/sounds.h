/*
	Notes are one note, Tunes are more that one note.
	To use these macros:
*/

#ifndef _SOUNDS_H_INCLUDED
#define _SOUNDS_H_INCLUDED

#include <unistd.h>
#ifdef __QNX__
#include <i86.h>
#else
#define delay(X)
#endif
#include <sound_esc.h>

#define DEFAULT_FREQUENCY 765

#define PASS_TUNE { \
	sound_esc(STDOUT_FILENO,850,250); \
	delay(250); \
	sound_esc(STDOUT_FILENO,750,250); \
	delay(250); \
	sound_esc(STDOUT_FILENO,850,250); }
#define WARN_TUNE { \
	sound_esc(STDOUT_FILENO,500,300); \
	delay(300); \
	sound_esc(STDOUT_FILENO,400,300); \
	delay(300); \
	sound_esc(STDOUT_FILENO,500,300); \
	delay(300); \
	sound_esc(STDOUT_FILENO,400,300); \
	delay(300); \
	sound_esc(STDOUT_FILENO,500,300); \
	delay(300); \
	sound_esc(STDOUT_FILENO,400,300); }
#define FAIL_TUNE { \
	sound_esc(STDOUT_FILENO,150,125); \
	delay(125); \
	sound_esc(STDOUT_FILENO,125,125); \
	delay(125); \
	sound_esc(STDOUT_FILENO,100,125); \
	delay(125); \
	sound_esc(STDOUT_FILENO,75,125); \
	delay(125); \
	sound_esc(STDOUT_FILENO,50,400); }
#define PASS_NOTE sound_esc(STDOUT_FILENO,850,200)
#define WARN_NOTE sound_esc(STDOUT_FILENO,500,300)
#define FAIL_NOTE sound_esc(STDOUT_FILENO,75,1000)

#endif

