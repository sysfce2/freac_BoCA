 /* BoCA - BonkEnc Component Architecture
  * Copyright (C) 2007-2013 Robert Kausch <robert.kausch@bonkenc.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#include <boca.h>

BoCA_BEGIN_COMPONENT(XSPFPlaylist)

namespace BoCA
{
	class XSPFPlaylist : public CS::PlaylistComponent
	{
		friend class XSPFPlaylistCallback;

		private:
			String			 EncodeURI(const String &);
			String			 DecodeURI(const String &);
		public:
			static const String	&GetComponentSpecs();

						 XSPFPlaylist();
						~XSPFPlaylist();

			Bool			 CanOpenFile(const String &);

			const Array<Track>	&ReadPlaylist(const String &);
			Error			 WritePlaylist(const String &);
	};
};

BoCA_DEFINE_PLAYLIST_COMPONENT(XSPFPlaylist)

BoCA_END_COMPONENT(XSPFPlaylist)