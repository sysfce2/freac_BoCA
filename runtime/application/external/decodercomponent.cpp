 /* BoCA - BonkEnc Component Architecture
  * Copyright (C) 2007-2022 Robert Kausch <robert.kausch@freac.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License as
  * published by the Free Software Foundation, either version 2 of
  * the License, or (at your option) any later version.
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#include <boca/application/external/decodercomponent.h>
#include <boca/application/external/configlayer.h>

#include <boca/application/registry.h>
#include <boca/application/taggercomponent.h>

#include <boca/common/utilities.h>

BoCA::AS::DecoderComponentExternal::DecoderComponentExternal(ComponentSpecs *specs) : DecoderComponent(specs)
{
	configuration	= NIL;

	configLayer	= NIL;

	packageSize	= 0;
	inBytes		= 0;
}

BoCA::AS::DecoderComponentExternal::~DecoderComponentExternal()
{
	if (configLayer != NIL) Object::DeleteObject(configLayer);
}

Bool BoCA::AS::DecoderComponentExternal::CanOpenStream(const String &streamURI)
{
	String	 lcURI = streamURI.ToLower();

	for (Int i = 0; i < specs->formats.Length(); i++)
	{
		FileFormat	*format = specs->formats.GetNth(i);

		for (Int j = 0; j < format->GetExtensions().Length(); j++)
		{
			if (!lcURI.EndsWith(String(".").Append(format->GetExtensions().GetNth(j)))) continue;

			if (GetStreamInfo(streamURI, track) == Success()) return True;
		}
	}

	return False;
}

Bool BoCA::AS::DecoderComponentExternal::SetAudioTrackInfo(const Track &track)
{
	this->track = track;

	return True;
}

File BoCA::AS::DecoderComponentExternal::GetCompanionFile(const String &file) const
{
	String	 companionExt = specs->formats.GetFirst()->GetCompanionExtensions().GetFirst();

	if (companionExt == NIL) return File();

	return file.Head(file.FindLast(".") + 1).Append(companionExt);
}

Int BoCA::AS::DecoderComponentExternal::SetDriver(IO::Driver *driver)
{
	return IO::Filter::SetDriver(driver);
}

Int BoCA::AS::DecoderComponentExternal::ProcessData(Buffer<UnsignedByte> &buffer)
{
	/* Find system byte order.
	 */
	static Int	 systemByteOrder = CPU().GetEndianness() == EndianLittle ? BYTE_INTEL : BYTE_RAW;

	/* Switch byte order to native.
	 */
	const Format	&format = track.GetFormat();

	if (format.order != BYTE_NATIVE && format.order != systemByteOrder) Utilities::SwitchBufferByteOrder(buffer, format.bits / 8);

	/* Calculate MD5 if requested.
	 */
	if (calculateMD5) md5.Feed(buffer);

	return buffer.Size();
}

BoCA::ConfigLayer *BoCA::AS::DecoderComponentExternal::GetConfigurationLayer()
{
	if (configLayer == NIL && specs->parameters.Length() > 0) configLayer = new ConfigLayerExternal(specs);

	return configLayer;
}

const BoCA::Config *BoCA::AS::DecoderComponentExternal::GetConfiguration() const
{
	if (configuration != NIL) return configuration;
	else			  return Config::Get();
}

Bool BoCA::AS::DecoderComponentExternal::SetConfiguration(const Config *nConfiguration)
{
	configuration = nConfiguration;

	return True;
}

Int BoCA::AS::DecoderComponentExternal::QueryTags(const String &streamURI, Track &track) const
{
	/* Loop over supported formats.
	 */
	String	 lcURI = streamURI.ToLower();

	foreach (FileFormat *format, specs->formats)
	{
		foreach (const String &extension, format->GetExtensions())
		{
			if (!lcURI.EndsWith(String(".").Append(extension))) continue;

			/* Read supported tag formats.
			 */
			const Array<TagFormat>	&tagFormats = format->GetTagFormats();

			foreach (const TagFormat &tagFormat, tagFormats)
			{
				AS::Registry		&boca	= AS::Registry::Get();
				AS::TaggerComponent	*tagger = (AS::TaggerComponent *) boca.CreateComponentByID(tagFormat.GetTagger());

				if (tagger != NIL)
				{
					tagger->SetConfiguration(GetConfiguration());
					tagger->ParseStreamInfo(streamURI, track);

					boca.DeleteComponent(tagger);
				}
			}

			/* Consider a track lossless if bitrate is high enough to
			 * avoid a lossy to lossless conversion warning on hybrid
			 * formats like WavPack.
			 */
			Bool	 lossless = format->IsLossless();

			if (!lossless && track.length > 0)
			{
				const Format	&format = track.GetFormat();

				Int	 rawBitrate  = format.channels * format.rate * format.bits;
				Int	 fileBitrate = Float(track.fileSize) / (Float(track.length) / format.rate) * 8.0;

				if (fileBitrate > rawBitrate * 0.33) lossless = True;
			}

			/* Set decoder ID and lossless flag for track and chapters.
			 */
			track.decoderID = specs->id;
			track.lossless	= lossless;

			foreach (Track &chapter, track.tracks)
			{
				if (chapter.decoderID != NIL) continue;

				chapter.decoderID = specs->id;
				chapter.lossless  = track.lossless;
			}

			break;
		}
	}

	return Success();
}
