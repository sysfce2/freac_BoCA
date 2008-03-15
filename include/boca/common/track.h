 /* BoCA - BonkEnc Component Architecture
  * Copyright (C) 2007-2008 Robert Kausch <robert.kausch@bonkenc.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#ifndef H_BOCA_TRACK
#define H_BOCA_TRACK

#include <smooth.h>
#include "picture.h"

using namespace smooth;

const int	 BYTE_INTEL	= 0;
const int	 BYTE_RAW	= 1;

namespace BoCA
{
	class BOCA_DLL_EXPORT Track
	{
		private:
			Bool		 ParseID3V2Tag(Void *);
			String		 GetID3V2FrameString(Void *);

			const String	&GetID3CategoryName(Int);

			String		 GetTempFileName(const String &);

			String		 CreateTempFile(const String &);
			Bool		 RemoveTempFile(const String &);
		public:
		    // Audio format information:
			Int		 channels;
			Int		 rate;
			Int		 bits;
			Int64		 length;
			Int64		 approxLength;
			Int64		 fileSize;
			Int		 order;

		    // CD track information:
			Bool		 isCDTrack;
			Int		 drive;
			Int		 cdTrack;

		    // Title information:
			String		 artist;
			String		 title;
			String		 album;
			Int		 track;
			String		 genre;
			Int		 year;
			String		 comment;

			String		 oArtist;
			String		 oTitle;
			String		 oAlbum;
			String		 oGenre;

		    // Attached pictures:
			Array<Picture>	 pictures;

		    // CDDB information:
			Int		 offset;
			String		 discid;
			String		 category;
			Int		 revision;
			Int		 disclength;
			String		 discComment;
			String		 playorder;

		    // Other information:
			String		 fileSizeString;
			String		 lengthString;

			String		 outfile;
			String		 origFilename;

					 Track();
					 Track(const Track &);
					~Track();

			Track &operator	 =(const Track &);

			Int		 RenderID3Tag(Int, Buffer<unsigned char> &);

			Bool		 ParseID3V2Tag(Buffer<unsigned char> &);
			Bool		 ParseID3V2Tag(const String &);
	};
};

#endif