 /* BoCA - BonkEnc Component Architecture
  * Copyright (C) 2007-2020 Robert Kausch <robert.kausch@freac.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License as
  * published by the Free Software Foundation, either version 2 of
  * the License, or (at your option) any later version.
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#ifndef H_CDRIP_CDPLAYER
#define H_CDRIP_CDPLAYER

#include <smooth.h>

using namespace smooth;

#include "cdinfo.h"

namespace BoCA
{
	class CDPlayerIni
	{
		private:
			static String	 DiscIDToString(UnsignedInt32);

			CDInfo		 cdInfo;
		public:
					 CDPlayerIni();
					~CDPlayerIni();

			Int		 ReadCDInfo(Int);

			const CDInfo	&GetCDInfo() const;
	};
};

#endif
