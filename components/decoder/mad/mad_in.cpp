 /* BoCA - BonkEnc Component Architecture
  * Copyright (C) 2007-2010 Robert Kausch <robert.kausch@bonkenc.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#include <smooth.h>
#include <smooth/dll.h>

#include <smooth/io/drivers/driver_posix.h>

#include "mad_in.h"
#include "dllinterface.h"
#include "xing/dxhead.h"

using namespace smooth::IO;

const String &BoCA::MADIn::GetComponentSpecs()
{
	static String	 componentSpecs;

	if (maddll != NIL)
	{
		componentSpecs = "						\
										\
		  <?xml version=\"1.0\" encoding=\"UTF-8\"?>			\
		  <component>							\
		    <name>MAD MP3 Decoder</name>				\
		    <version>1.0</version>					\
		    <id>mad-in</id>						\
		    <type>decoder</type>					\
		    <format>							\
		      <name>MPEG Audio Files</name>				\
		      <extension>mp1</extension>				\
		      <extension>mp2</extension>				\
		      <extension>mp3</extension>				\
		      <tag id=\"id3v2-tag\" mode=\"prepend\">ID3v2</tag>	\
		    </format>							\
		  </component>							\
										\
		";
	}

	return componentSpecs;
}

Void smooth::AttachDLL(Void *instance)
{
	LoadMADDLL();
}

Void smooth::DetachDLL()
{
	FreeMADDLL();
}

namespace BoCA
{
	mad_flow	 MADInputCallback(void *, mad_stream *);
	mad_flow	 MADOutputCallback(void *, const mad_header *, mad_pcm *);
	mad_flow	 MADHeaderCallback(void *, const mad_header *, mad_pcm *);
	mad_flow	 MADErrorCallback(void *, mad_stream *, mad_frame *);

	/* FIXME: This is the scaling function included in the MAD
	 *	  package. It should be replaced by a more decent one.
	 */
	static inline signed int scale(mad_fixed_t sample)
	{
		/* Round
		 */
		sample += (1L << (MAD_F_FRACBITS - 16));

		/* Clip
		 */
		if	(sample >= MAD_F_ONE) sample = MAD_F_ONE - 1;
		else if (sample < -MAD_F_ONE) sample = -MAD_F_ONE;

		/* Quantize
		 */
		return sample >> (MAD_F_FRACBITS + 1 - 16);
	}
};

Bool BoCA::MADIn::CanOpenStream(const String &streamURI)
{
	String	 lcURI = streamURI.ToLower();

	return lcURI.EndsWith(".mp1") ||
	       lcURI.EndsWith(".mp2") ||
	       lcURI.EndsWith(".mp3");
}

Error BoCA::MADIn::GetStreamInfo(const String &streamURI, Track &track)
{
	Driver		*ioDriver = new DriverPOSIX(streamURI, IS_READ);
	InStream	*f_in = new InStream(STREAM_DRIVER, ioDriver);

	track.fileSize	= f_in->Size();
	track.length	= -1;

	infoTrack = &track;
	stop = False;

	SkipID3v2Tag(f_in);
	ReadXingTag(f_in);

	driver = ioDriver;
	driver->Seek(f_in->GetPos());

	readDataMutex = new Mutex();
	samplesBufferMutex = new Mutex();

	decoderThread = NonBlocking1<Bool>(&MADIn::ReadMAD, this).Call(False);

	while (decoderThread->GetStatus() == THREAD_RUNNING) S::System::System::Sleep(0);

	delete readDataMutex;
	delete samplesBufferMutex;

	delete f_in;
	delete ioDriver;

	if (!errorState)
	{
		Bool			 foundTag = False;

		AS::Registry		&boca = AS::Registry::Get();
		AS::TaggerComponent	*tagger = (AS::TaggerComponent *) boca.CreateComponentByID("id3v2-tag");

		if (tagger != NIL)
		{
			if (tagger->ParseStreamInfo(streamURI, track) == Success()) foundTag = True;

			boca.DeleteComponent(tagger);
		}

		if (!foundTag)
		{
			tagger = (AS::TaggerComponent *) boca.CreateComponentByID("id3v1-tag");

			if (tagger != NIL)
			{
				tagger->ParseStreamInfo(streamURI, track);

				boca.DeleteComponent(tagger);
			}
		}
	}

	return Success();
}

BoCA::MADIn::MADIn()
{
	packageSize = 0;

	infoTrack = NIL;
	numFrames = 0;
}

BoCA::MADIn::~MADIn()
{
}

Bool BoCA::MADIn::Activate()
{
	stop = False;

	InStream	*f_in = new InStream(STREAM_DRIVER, driver);

	SkipID3v2Tag(f_in);

	driver->Seek(f_in->GetPos());

	delete f_in;

	readDataMutex = new Mutex();
	samplesBufferMutex = new Mutex();

	readDataMutex->Lock();

	decoderThread = NIL;

	return True;
}

Bool BoCA::MADIn::Deactivate()
{
	if (decoderThread != NIL)
	{
		stop = True;

		readDataMutex->Release();

		while (decoderThread->GetStatus() == THREAD_RUNNING) S::System::System::Sleep(0);
	}

	delete readDataMutex;
	delete samplesBufferMutex;

	return True;
}

Int BoCA::MADIn::ReadData(Buffer<UnsignedByte> &data, Int size)
{
	if (decoderThread == NIL) decoderThread = NonBlocking1<Bool>(&MADIn::ReadMAD, this).Call(True);

	if (decoderThread->GetStatus() != THREAD_RUNNING && samplesBuffer.Size() <= 0) return -1;

	readDataMutex->Release();

	while (decoderThread->GetStatus() == THREAD_RUNNING && samplesBuffer.Size() <= 0) S::System::System::Sleep(0);

	readDataMutex->Lock();

	samplesBufferMutex->Lock();

	size = samplesBuffer.Size() * (track.GetFormat().bits / 8);

	data.Resize(size);

	for (Int i = 0; i < samplesBuffer.Size(); i++) ((Short *) (unsigned char *) data)[i] = scale(samplesBuffer[i]);

	samplesBuffer.Resize(0);

	samplesBufferMutex->Release();

	return size;
}

Bool BoCA::MADIn::SkipID3v2Tag(InStream *in)
{
	/* Check for an ID3v2 tag at the beginning of the
	 * file and skip it if it exists as it might cause
	 * problems if the tag is unsynchronized.
	 */
	if (in->InputString(3) == "ID3")
	{
		in->InputNumber(2); // ID3 version
		in->InputNumber(1); // Flags

		/* Read tag size as a 4 byte unsynchronized integer.
		 */
		Int	 tagSize = (in->InputNumber(1) << 21) +
				   (in->InputNumber(1) << 14) +
				   (in->InputNumber(1) <<  7) +
				   (in->InputNumber(1)      );

		in->RelSeek(tagSize);

		inBytes += (tagSize + 10);
	}
	else
	{
		in->Seek(0);
	}

	return True;
}

Bool BoCA::MADIn::ReadXingTag(InStream *in)
{
	/* Check for a Xing header and extract
	 * the number of samples if it exists.
	 */

	Buffer<UnsignedByte>	 buffer(156);

	/* Read data and seek back to before
	 * the Xing header.
	 */
	in->InputData(buffer, 156);
	in->RelSeek(-156);

	XHEADDATA		 data;

	data.toc = NIL;

	if (GetXingHeader(&data, buffer))
	{
		numFrames = data.frames;

		return True;
	}

	return False;
}

Int BoCA::MADIn::ReadMAD(Bool readData)
{
	if (readData)	ex_mad_decoder_init(&decoder, this, &MADInputCallback, NIL, NIL, &MADOutputCallback, &MADErrorCallback, NIL);
	else		ex_mad_decoder_init(&decoder, this, &MADInputCallback, NIL, NIL, &MADHeaderCallback, &MADErrorCallback, NIL);

	ex_mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

	ex_mad_decoder_finish(&decoder);

	return Success();
}

mad_flow BoCA::MADInputCallback(void *client_data, mad_stream *stream)
{
	MADIn	*filter = (MADIn *) client_data;

	if (filter->stop || filter->driver->GetPos() == filter->driver->GetSize()) return MAD_FLOW_STOP;

	static Buffer<UnsignedByte>	 inputBuffer;

	filter->readDataMutex->Lock();

	Int	 bytes = Math::Min(10000, filter->driver->GetSize() - filter->driver->GetPos());
	Int	 backup = stream->bufend - stream->next_frame;

	inputBuffer.Resize(bytes + backup);

	memmove(inputBuffer, stream->next_frame, backup);

	filter->driver->ReadData(inputBuffer + backup, bytes);

	filter->readDataMutex->Release();

	ex_mad_stream_buffer(stream, inputBuffer, bytes + backup);

	filter->inBytes += bytes;

	return MAD_FLOW_CONTINUE;
}

mad_flow BoCA::MADOutputCallback(void *client_data, const mad_header *header, mad_pcm *pcm)
{
	MADIn	*filter = (MADIn *) client_data;

	filter->samplesBufferMutex->Lock();

	Int	 oSize = filter->samplesBuffer.Size();
	Int	 channels = header->mode == MAD_MODE_SINGLE_CHANNEL ? 1 : 2;

	filter->samplesBuffer.Resize(oSize + pcm->length * channels);

	for (Int i = 0; i < (signed) pcm->length; i++)
	{
		for (Int j = 0; j < channels; j++)
		{
			filter->samplesBuffer[oSize + i * channels + j] = pcm->samples[j][i];
		}
	}

	filter->samplesBufferMutex->Release();

	return MAD_FLOW_CONTINUE;
}

mad_flow BoCA::MADHeaderCallback(void *client_data, const mad_header *header, mad_pcm *pcm)
{
	MADIn	*filter = (MADIn *) client_data;

	Format	 format = filter->infoTrack->GetFormat();

	format.bits	= 16;
	format.order	= BYTE_INTEL;
	format.channels	= header->mode == MAD_MODE_SINGLE_CHANNEL ? 1 : 2;
	format.rate	= header->samplerate;

	filter->infoTrack->approxLength = filter->infoTrack->fileSize / (header->bitrate / 8) * format.rate * format.channels;

	filter->infoTrack->SetFormat(format);

	/* If we previously found a Xing header,
	 * we can compute the exact duration from
	 * the number of frames in the file.
	 */
	if (filter->numFrames > 0)
	{
		filter->infoTrack->length = filter->numFrames * Math::Round((Float) header->duration.fraction / MAD_TIMER_RESOLUTION * format.rate) * format.channels;
	}

	return MAD_FLOW_STOP;
}

mad_flow BoCA::MADErrorCallback(void *client_data, mad_stream *stream, mad_frame *frame)
{
	return MAD_FLOW_CONTINUE;
}