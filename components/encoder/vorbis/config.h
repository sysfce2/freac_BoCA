 /* BoCA - BonkEnc Component Architecture
  * Copyright (C) 2007-2017 Robert Kausch <robert.kausch@freac.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License as
  * published by the Free Software Foundation, either version 2 of
  * the License, or (at your option) any later version.
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#ifndef H_VORBISCONFIG
#define H_VORBISCONFIG

#include <smooth.h>
#include <boca.h>

using namespace smooth;
using namespace smooth::GUI;

using namespace BoCA;

namespace BoCA
{
	class ConfigureVorbis : public ConfigLayer
	{
		private:
			GroupBox		*group_mode;
			OptionBox		*option_mode_vbr;
			OptionBox		*option_mode_abr;

			GroupBox		*group_extension;
			OptionBox		*option_extension_ogg;
			OptionBox		*option_extension_oga;

			GroupBox		*group_quality;
			Slider			*slider_quality;
			Text			*text_quality;
			Text			*text_quality_value;

			GroupBox		*group_bitrate;
			CheckBox		*check_abrmin;
			Slider			*slider_abrmin;
			EditBox			*edit_abrmin;
			Text			*text_abrmin_kbps;
			CheckBox		*check_abrnom;
			Slider			*slider_abrnom;
			EditBox			*edit_abrnom;
			Text			*text_abrnom_kbps;
			CheckBox		*check_abrmax;
			Slider			*slider_abrmax;
			EditBox			*edit_abrmax;
			Text			*text_abrmax_kbps;

			Int			 mode;
			Int			 fileExtension;
			Int			 quality;
			Int			 abrMin;
			Int			 abrNom;
			Int			 abrMax;

			Bool			 setABRMin;
			Bool			 setABRNom;
			Bool			 setABRMax;
		slots:
			Void			 SetMode();
			Void			 SetQuality();

			Void			 ToggleABRMin();
			Void			 SetABRMin();
			Void			 SetABRMinByEditBox();

			Void			 ToggleABRNom();
			Void			 SetABRNom();
			Void			 SetABRNomByEditBox();

			Void			 ToggleABRMax();
			Void			 SetABRMax();
			Void			 SetABRMaxByEditBox();
		public:
			static const String	 ConfigID;

						 ConfigureVorbis();
						~ConfigureVorbis();

			Int			 SaveSettings();
	};
};

#endif
