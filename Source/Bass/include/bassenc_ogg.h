/*
	BASSenc_OGG 2.4 C/C++ header file
	Copyright (c) 2016 Un4seen Developments Ltd.

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

HENCODE BASSENCOGGDEF(BASS_Encode_OGG_Start)(DWORD handle, const char *options, DWORD flags, ENCODEPROC *proc, void *user);
HENCODE BASSENCOGGDEF(BASS_Encode_OGG_StartFile)(DWORD handle, const char *options, DWORD flags, const char *filename);

#ifdef __cplusplus
}

#ifdef _WIN32
static inline HENCODE BASS_Encode_StartOGG(DWORD handle, const WCHAR *options, DWORD flags, ENCODEPROC *proc, void *user)
{
	return BASS_Encode_OGG_Start(handle, (const char*)options, flags|BASS_UNICODE, proc, user);
}

static inline HENCODE BASS_Encode_StartOGGFile(DWORD handle, const WCHAR *options, DWORD flags, const WCHAR *filename)
{
	return BASS_Encode_OGG_StartFile(handle, (const char*)options, flags|BASS_UNICODE, (const char*)filename);
}
#endif
#endif

#endif
