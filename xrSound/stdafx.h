// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#ifndef stdafxH
#define stdafxH
#pragma once
#pragma warning(disable:4577)

#include "../xrCore/xrCore.h"

// mmsystem.h
#define MMNOSOUND
#define MMNOMIDI
#define MMNOAUX
#define MMNOMIXER
#define MMNOJOY
#include <mmsystem.h>

// mmreg.h
#define NOMMIDS
#define NONEWRIFF
#define NOJPEGDIB
#define NONEWIC
#define NOBITMAP
#include <mmreg.h>

#include "..\components\libvorbis\include\vorbis\codec.h"
#include "..\components\libvorbis\include\vorbis\vorbisfile.h"

#include "../xrCDB/xrCDB.h"
#include "sound.h"

#define ENGINE_API

#include "..\xrCore\xr_resource.h"

#ifdef _EDITOR    
# 	include "ETools.h"
#endif
// TODO: reference additional headers your program requires here
#endif

