// $Id$

// id3lib: a C++ library for creating and manipulating id3v1/v2 tags
// Copyright 1999, 2000  Scott Thomas Haug
// Copyright 2002  Thijmen Klok (thijmen@id3lib.org)

/* This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* The id3lib authors encourage improvements and optimisations to be sent to
 * the id3lib coordinator.  Please see the README file for details on where to
 * send such submissions.  See the AUTHORS file for a list of people who have
 * contributed to id3lib.  See the ChangeLog file for a list of changes to
 * id3lib.  These files are distributed with id3lib at
 * http://download.sourceforge.net/id3lib/
 */

#include "tag_impl.h"

#include <id3/helpers.h>
#include <id3/io_strings.h>

#if defined HAVE_SYS_PARAM_H
#	include <sys/param.h>
#endif

#define ID3_PADMULTIPLE (2048)
#define ID3_PADMAX (4096)

using namespace dami;

void id3::v1::render(ID3_Writer &writer, const ID3_TagImpl &tag)
{
	writer.writeChars("TAG", 3);

	io::writeTrailingSpaces(writer, id3::v2::getTitle(tag),  ID3_V1_LEN_TITLE);
	io::writeTrailingSpaces(writer, id3::v2::getArtist(tag), ID3_V1_LEN_ARTIST);
	io::writeTrailingSpaces(writer, id3::v2::getAlbum(tag),  ID3_V1_LEN_ALBUM);
	io::writeTrailingSpaces(writer, id3::v2::getYear(tag),   ID3_V1_LEN_YEAR);

	size_t	 track = id3::v2::getTrackNum(tag);
	String	 comment = id3::v2::getV1Comment(tag);

	if (track > 0)
	{
		io::writeTrailingSpaces(writer, comment, ID3_V1_LEN_COMMENT - 2);

		writer.writeChar('\0');
		writer.writeChar((char) track);
	}
	else
	{
		io::writeTrailingSpaces(writer, comment, ID3_V1_LEN_COMMENT);
	}

	writer.writeChar((char) id3::v2::getGenreNum(tag));
}

ID3_Err id3::v2::render(ID3_Writer &writer, const ID3_TagImpl &tag)
{
	/* There has to be at least one frame for there to be a tag...
	 */
	ID3_Err err = ID3E_NoError;
	if (tag.NumFrames() == 0)
	{
		ID3D_WARNING("id3::v2::render(): no frames to render");

		return ID3E_NoData;
	}

	ID3_V2Spec	 spec = tag.MinSpec();

	if (tag.GetSpec() > spec) spec = tag.GetSpec();

	ID3D_NOTICE("id3::v2::render(): rendering");

	ID3_TagHeader	 hdr;

	hdr.SetSpec(spec);
	hdr.SetExtended(tag.GetExtended());
	hdr.SetExperimental(tag.GetExperimental());
	hdr.SetFooter(tag.GetFooter());

	/* Set up the encryption and grouping IDs.
	 */

	/* ...
	 */
	String			 frms;
	io::StringWriter	 frmWriter(frms);

	if (!tag.GetUnsync())
	{
		ID3D_NOTICE("id3::v2::render(): rendering frames");

		err = renderFrames(frmWriter, tag);

		if (err != ID3E_NoError) return err;

		hdr.SetUnsync(false);
	}
	else
	{
		ID3D_NOTICE("id3::v2::render(): rendering unsynced frames");

		io::UnsyncedWriter	 uw(frmWriter);

		err = renderFrames(uw, tag);

		if (err != ID3E_NoError) return err;

		uw.flush();

		ID3D_NOTICE("id3::v2::render(): numsyncs = " << uw.getNumSyncs());

		hdr.SetUnsync(uw.getNumSyncs() > 0);
	}

	size_t	 frmSize = frms.size();

	if (frmSize == 0)
	{
		ID3D_WARNING("id3::v2::render(): rendered frame size is 0 bytes");

		return ID3E_InvalidFrameSize;
	}

	/* Zero the remainder of the buffer so that our padding bytes are zero.
	 */
	luint	 nPadding = tag.PaddingSize(frmSize);

	ID3D_NOTICE("id3::v2::render(): padding size = " << nPadding);

	hdr.SetDataSize(frmSize + tag.GetExtendedBytes() + nPadding);

	err = hdr.Render(writer);

	if (err != ID3E_NoError) return err;

	writer.writeChars(frms.data(), frms.size());

	for (size_t i = 0; i < nPadding; ++i)
	{
		if (writer.writeChar('\0') == ID3_Writer::END_OF_WRITER)
		{
			break;
		}
	}

	return ID3E_NoError;
}

size_t ID3_TagImpl::Size() const
{
	if (this->NumFrames() == 0) return 0;

	ID3_V2Spec	 spec = this->MinSpec();

	if (this->GetSpec() > spec) spec = this->GetSpec();

	ID3_TagHeader hdr;

	hdr.SetSpec(spec);

	size_t	 bytesUsed = hdr.Size();
	size_t	 frameBytes = ID3_ContainerImpl::Size();

	if (!frameBytes) return 0;

	bytesUsed += frameBytes;

	/* Add 30% for sync.
	 */
	bytesUsed += bytesUsed / 3;

	/* Add padding and another ID3_PADMULTIPLE bytes
	 * to be safe in case the above guess was too low.
	 */
	bytesUsed += this->PaddingSize(bytesUsed) + ID3_PADMULTIPLE;

	return bytesUsed;
}

size_t ID3_TagImpl::PaddingSize(size_t curSize) const
{
	luint	 newSize = 0;

	/* If padding is switched off.
	 */
	if (!_is_padded) return 0;

	/* If the old tag was large enough to hold the new tag, then we will simply
	 * pad out the difference - that way the new tag can be written without
	 * shuffling the rest of the song file around.
	 */
	if ((this->GetPrependedBytes()-ID3_TagHeader::SIZE > 0) &&
	    (this->GetPrependedBytes()-ID3_TagHeader::SIZE >= curSize) &&
	    (this->GetPrependedBytes()-ID3_TagHeader::SIZE - curSize) < ID3_PADMAX)
	{
		newSize = this->GetPrependedBytes() - ID3_TagHeader::SIZE;
	}
	else
	{
		luint	 tempSize = curSize + ID3_GetDataSize(*this) + this->GetAppendedBytes() + ID3_TagHeader::SIZE;

		/* This method of automatic padding rounds the COMPLETE FILE up to the
		 * nearest 2K.  If the file will already be an even multiple of 2K (with
		 * the tag included) then we just add another 2K of padding.
		 */
		tempSize = ((tempSize / ID3_PADMULTIPLE) + 1) * ID3_PADMULTIPLE;

		/* The size of the new tag is the new filesize minus the audio data.
		 */
		newSize = tempSize - ID3_GetDataSize(*this) - this->GetAppendedBytes () - ID3_TagHeader::SIZE;
	}

	return newSize - curSize;
}
