/*
	BASSenc_OGG 2.4 C/C++ header file
	Copyright (c) 2016-2020 Un4seen Developments Ltd.

	See the BASSENC_OGG.CHM file for more detailed documentation
*/

#ifndef BASSENC_OGG_H
#define BASSENC_OGG_H

#include "bassenc.h"

#if BASSVERSION!=0x204
#error conflicting BASS and BASSenc_OGG versions
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BASSENCOGGDEF
#define BASSENCOGGDEF(f) WINAPI f
#endif

// BASS_Encode_OGG_NewStream flags
#define BASS_ENCODE_OGG_RESET		0x1000000

DWORD BASSENCOGGDEF(BASS_Encode_OGG_GetVersion)(void);

HENCODE BASSENCOGGDEF(BASS_Encode_OGG_Start)(DWORD handle, const char *options, DWORD flags, ENCODEPROC *proc, void *user);
HENCODE BASSENCOGGDEF(BASS_Encode_OGG_StartFile)(DWORD handle, const char *options, DWORD flags, const char *filename);
BOOL BASSENCOGGDEF(BASS_Encode_OGG_NewStream)(HENCODE handle, const char *options, DWORD flags);

#ifdef __cplusplus
}

#ifdef _WIN32
static inline HENCODE BASS_Encode_OGG_Start(DWORD handle, const WCHAR *options, DWORD flags, ENCODEPROC *proc, void *user)
{
	return BASS_Encode_OGG_Start(handle, (const char*)options, flags | BASS_UNICODE, proc, user);
}

static inline HENCODE BASS_Encode_OGG_StartFile(DWORD handle, const WCHAR *options, DWORD flags, const WCHAR *filename)
{
	return BASS_Encode_OGG_StartFile(handle, (const char*)options, flags | BASS_UNICODE, (const char*)filename);
}

static inline BOOL BASS_Encode_OGG_NewStream(HENCODE handle, const WCHAR *options, DWORD flags)
{
	return BASS_Encode_OGG_NewStream(handle, (const char*)options, flags | BASS_UNICODE);
}
#endif
#endif

#endif
