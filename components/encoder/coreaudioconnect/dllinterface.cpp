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

#include <boca.h>
#include "dllinterface.h"

using namespace BoCA;

MP4MODIFY			 ex_MP4Modify			= NIL;
MP4CLOSE			 ex_MP4Close			= NIL;
MP4OPTIMIZE			 ex_MP4Optimize			= NIL;

MP4FINDTRACKID			 ex_MP4FindTrackId		= NIL;
MP4SETTRACKINTEGERPROPERTY	 ex_MP4SetTrackIntegerProperty	= NIL;

DynamicLoader *mp4v2dll = NIL;

Bool LoadMP4v2DLL()
{
	mp4v2dll = BoCA::Utilities::LoadCodecDLL("mp4v2");

	if (mp4v2dll == NIL) return False;

	ex_MP4Modify			= (MP4MODIFY) mp4v2dll->GetFunctionAddress("MP4Modify");
	ex_MP4Close			= (MP4CLOSE) mp4v2dll->GetFunctionAddress("MP4Close");
	ex_MP4Optimize			= (MP4OPTIMIZE) mp4v2dll->GetFunctionAddress("MP4Optimize");

	ex_MP4FindTrackId		= (MP4FINDTRACKID) mp4v2dll->GetFunctionAddress("MP4FindTrackId");
	ex_MP4SetTrackIntegerProperty	= (MP4SETTRACKINTEGERPROPERTY) mp4v2dll->GetFunctionAddress("MP4SetTrackIntegerProperty");

	if (ex_MP4Modify			== NIL ||
	    ex_MP4Close				== NIL ||
	    ex_MP4Optimize			== NIL ||

	    ex_MP4FindTrackId			== NIL ||
	    ex_MP4SetTrackIntegerProperty	== NIL) { FreeMP4v2DLL(); return False; }

	return True;
}

Void FreeMP4v2DLL()
{
	BoCA::Utilities::FreeCodecDLL(mp4v2dll);

	mp4v2dll = NIL;
}
