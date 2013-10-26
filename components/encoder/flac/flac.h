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
#include "dllinterface.h"

BoCA_BEGIN_COMPONENT(EncoderFLAC)

namespace BoCA
{
	class EncoderFLAC : public CS::EncoderComponent
	{
		friend FLAC__StreamEncoderReadStatus	 FLACStreamEncoderReadCallback(const FLAC__StreamEncoder *, FLAC__byte[], size_t *, void *);
		friend FLAC__StreamEncoderWriteStatus	 FLACStreamEncoderWriteCallback(const FLAC__StreamEncoder *, const FLAC__byte[], size_t, unsigned, unsigned, void *);
		friend FLAC__StreamEncoderSeekStatus	 FLACStreamEncoderSeekCallback(const FLAC__StreamEncoder *, FLAC__uint64, void *);
		friend FLAC__StreamEncoderTellStatus	 FLACStreamEncoderTellCallback(const FLAC__StreamEncoder *, FLAC__uint64 *, void *);

		private:
			ConfigLayer			*configLayer;

			FLAC__StreamEncoder		*encoder;

			Array<FLAC__StreamMetadata *>	 metadata;

			Buffer<FLAC__int32>		 buffer;
			Int				 bytesWritten;
		public:
			static const String		&GetComponentSpecs();

							 EncoderFLAC();
							~EncoderFLAC();

			Bool				 Activate();
			Bool				 Deactivate();

			Int				 WriteData(Buffer<UnsignedByte> &, Int);

			String				 GetOutputFileExtension();

			ConfigLayer			*GetConfigurationLayer();
	};
};

BoCA_DEFINE_ENCODER_COMPONENT(EncoderFLAC)

BoCA_END_COMPONENT(EncoderFLAC)