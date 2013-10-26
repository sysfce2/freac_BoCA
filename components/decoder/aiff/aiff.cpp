 /* BoCA - BonkEnc Component Architecture
  * Copyright (C) 2007-2013 Robert Kausch <robert.kausch@bonkenc.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#include <stdint.h>
#include <math.h>

#include "aiff.h"

using namespace smooth::IO;

const String &BoCA::DecoderAIFF::GetComponentSpecs()
{
	static String	 componentSpecs = "				\
									\
	  <?xml version=\"1.0\" encoding=\"UTF-8\"?>			\
	  <component>							\
	    <name>Apple Audio File Decoder</name>			\
	    <version>1.0</version>					\
	    <id>aiff-dec</id>						\
	    <type>decoder</type>					\
	    <format>							\
	      <name>Apple Audio Files</name>				\
	      <extension>aif</extension>				\
	      <extension>aiff</extension>				\
	      <extension>aifc</extension>				\
	      <tag id=\"id3v2-tag\" mode=\"other\">ID3v2</tag>		\
	      <tag id=\"tocplist-tag\" mode=\"other\">.TOC.plist</tag>	\
	    </format>							\
	  </component>							\
									\
	";

	return componentSpecs;
}

Bool BoCA::DecoderAIFF::CanOpenStream(const String &streamURI)
{
	InStream	 in(STREAM_FILE, streamURI, IS_READ);

	if (in.InputString(4) != "FORM") return False;

	in.RelSeek(4);

	String	 fileType = in.InputString(4);

	if	(fileType == "AIFF") return True;
	else if (fileType != "AIFC") return False;

	String		 chunk;

	while (chunk != "COMM")
	{
		if (in.GetPos() >= in.Size()) break;

		/* Read next chunk.
		 */
		chunk = in.InputString(4);

		Int	 cSize = in.InputNumberRaw(4);

		if (chunk == "COMM")
		{
			in.RelSeek(18);

			/* Parse AIFF-C compression type.
			 */
			String	 compression = in.InputString(4);

			if (compression == "NONE" || compression == "sowt") return True;

			/* Skip rest of chunk.
			 */
			in.RelSeek(cSize - 22 + cSize % 2);
		}
		else
		{
			/* Skip chunk.
			 */
			in.RelSeek(cSize + cSize % 2);
		}
	}

	return False;
}

Error BoCA::DecoderAIFF::GetStreamInfo(const String &streamURI, Track &track)
{
	InStream	*f_in = new InStream(STREAM_FILE, streamURI, IS_READ);

	track.fileSize	= f_in->Size();

	/* Read FORM chunk.
	 */
	if (f_in->InputString(4) != "FORM") { errorState = True; errorString = "Unknown file type"; }

	f_in->RelSeek(4);

	String	 fileType = f_in->InputString(4);

	if (fileType != "AIFF" && fileType != "AIFC") { errorState = True; errorString = "Unknown file type"; }

	String		 chunk;

	while (!errorState)
	{
		if (f_in->GetPos() >= f_in->Size()) break;

		/* Read next chunk.
		 */
		chunk = f_in->InputString(4);

		Int	 cSize = f_in->InputNumberRaw(4);

		if (chunk == "COMM")
		{
			Int	 cStart = f_in->GetPos();

			Format	 format = track.GetFormat();

			format.channels	= (unsigned short) f_in->InputNumberRaw(2);
			track.length	= (unsigned long) f_in->InputNumberRaw(4);

			format.order	= BYTE_RAW;
			format.bits	= (unsigned short) f_in->InputNumberRaw(2);

			/* Read sample rate as 80 bit float.
			 */
			int16_t		 exp = (f_in->InputNumberRaw(2) & 0x7FFF) - 16383 - 63;
			uint64_t	 man = 0;

			for (int i = 0; i < 8; i++) man = (man << 8) | f_in->InputNumberRaw(1);

			format.rate	= ldexp((double) man, exp);

			/* Parse AIFF-C compression type.
			 */
			if (fileType == "AIFC")
			{
				String	 compression = f_in->InputString(4);

				if	(compression == "NONE") format.order = BYTE_RAW;
				else if (compression == "sowt") format.order = BYTE_INTEL;
				else				{ errorState = True; errorString = "Unsupported audio format"; }
			}

			track.SetFormat(format);

			/* Skip rest of chunk.
			 */
			f_in->Seek(cStart + cSize + cSize % 2);
		}
		else if (chunk == "AUTH" ||
			 chunk == "NAME" ||
			 chunk == "ANNO")
		{
			Info	 info = track.GetInfo();

			if	(chunk == "AUTH") info.artist  = f_in->InputString(cSize);
			else if	(chunk == "NAME") info.title   = f_in->InputString(cSize);
			else if	(chunk == "ANNO") info.comment = f_in->InputString(cSize);

			track.SetInfo(info);

			/* Skip rest of chunk.
			 */
			f_in->RelSeek(cSize % 2);
		}
		else if (chunk == "ID3 ")
		{
			Buffer<UnsignedByte>	 buffer(cSize);

			f_in->InputData(buffer, cSize);

			AS::Registry		&boca = AS::Registry::Get();
			AS::TaggerComponent	*tagger = (AS::TaggerComponent *) boca.CreateComponentByID("id3v2-tag");

			if (tagger != NIL)
			{
				tagger->ParseBuffer(buffer, track);

				boca.DeleteComponent(tagger);
			}

			/* Skip rest of chunk.
			 */
			f_in->RelSeek(cSize % 2);
		}
		else
		{
			/* Skip chunk.
			 */
			f_in->RelSeek(cSize + cSize % 2);
		}
	}

	delete f_in;

	/* Read .TOC.plist if it exists.
	 */
	AS::Registry		&boca = AS::Registry::Get();
	AS::TaggerComponent	*tagger = (AS::TaggerComponent *) boca.CreateComponentByID("tocplist-tag");

	if (tagger != NIL)
	{
		tagger->ParseStreamInfo(streamURI, track);

		boca.DeleteComponent(tagger);
	}

	if (errorState)	return Error();
	else		return Success();
}

BoCA::DecoderAIFF::DecoderAIFF()
{
	packageSize = 0;

	dataOffset  = 0;
}

BoCA::DecoderAIFF::~DecoderAIFF()
{
}

Bool BoCA::DecoderAIFF::Activate()
{
	InStream	*in = new InStream(STREAM_DRIVER, driver);
    
	in->Seek(12);

	String		 chunk;

	do
	{
		/* Read next chunk
		 */
		chunk = in->InputString(4);

		Int	 cSize = in->InputNumberRaw(4);

		if (chunk != "SSND") in->RelSeek(cSize + cSize % 2);
	}
	while (chunk != "SSND");

	dataOffset = in->GetPos() + 8;

	delete in;

	driver->Seek(dataOffset);

	return True;
}

Bool BoCA::DecoderAIFF::Deactivate()
{
	return True;
}

Bool BoCA::DecoderAIFF::Seek(Int64 samplePosition)
{
	driver->Seek(dataOffset + samplePosition * track.GetFormat().channels * (track.GetFormat().bits / 8));

	return True;
}

Int BoCA::DecoderAIFF::ReadData(Buffer<UnsignedByte> &data, Int size)
{
	if (driver->GetPos() == driver->GetSize()) return -1;

	data.Resize(size);

	size = driver->ReadData(data, size);

	/* Convert 8 bit samples to unsigned.
	 */
	if (track.GetFormat().bits == 8) for (Int i = 0; i < size; i++) data[i] = data[i] + 128;

	return size;
}