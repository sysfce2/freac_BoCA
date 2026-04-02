 /* BoCA - BonkEnc Component Architecture
  * Copyright (C) 2007-2026 Robert Kausch <robert.kausch@freac.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License as
  * published by the Free Software Foundation, either version 2 of
  * the License, or (at your option) any later version.
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#include <smooth.h>

#ifdef callbacks
#	undef callbacks
#endif

#include <mp4v2/mp4v2.h>

using namespace smooth;
using namespace smooth::System;

extern DynamicLoader	*mp4v2dll;

Bool			 LoadMP4v2DLL();
Void			 FreeMP4v2DLL();

typedef MP4FileHandle			(*MP4MODIFY)			(const char *, uint32_t);
typedef void				(*MP4CLOSE)			(MP4FileHandle, uint32_t);
typedef bool				(*MP4OPTIMIZE)			(const char *, const char *);

typedef MP4TrackId			(*MP4FINDTRACKID)		(MP4FileHandle, uint16_t, const char *, uint8_t);
typedef bool				(*MP4SETTRACKINTEGERPROPERTY)	(MP4FileHandle, MP4TrackId, const char *, int64_t);

extern MP4MODIFY			 ex_MP4Modify;
extern MP4CLOSE				 ex_MP4Close;
extern MP4OPTIMIZE			 ex_MP4Optimize;

extern MP4FINDTRACKID			 ex_MP4FindTrackId;
extern MP4SETTRACKINTEGERPROPERTY	 ex_MP4SetTrackIntegerProperty;
