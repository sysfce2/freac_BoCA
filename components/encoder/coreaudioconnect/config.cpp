 /* BoCA - BonkEnc Component Architecture
  * Copyright (C) 2007-2014 Robert Kausch <robert.kausch@bonkenc.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#include "config.h"

BoCA::ConfigureCoreAudio::ConfigureCoreAudio(const CoreAudioCommCodecs &iFormats)
{
	Config	*config = Config::Get();

	formats		= iFormats;

	bitrate		= config->GetIntValue("CoreAudio", "Bitrate", 128);
	allowID3	= config->GetIntValue("CoreAudio", "AllowID3v2", 0);
	fileFormat	= config->GetIntValue("CoreAudio", "MP4Container", 1);
	fileExtension	= config->GetIntValue("CoreAudio", "MP4FileExtension", 0);

	I18n	*i18n = I18n::Get();

	tabwidget		= new TabWidget(Point(7, 7), Size(500, 208));

	i18n->SetContext("Encoders::AAC::Format");

	layer_format		= new Layer(i18n->TranslateString("Format"));

	group_id3v2		= new GroupBox(i18n->TranslateString("Tags"), Point(7, 88), Size(279, 90));

	check_id3v2		= new CheckBox(i18n->TranslateString("Allow ID3v2 tags in AAC files"), Point(10, 13), Size(200, 0), &allowID3);
	check_id3v2->SetWidth(check_id3v2->GetUnscaledTextWidth() + 20);

	text_note		= new Text(i18n->AddColon(i18n->TranslateString("Note")), Point(10, 38));
	text_id3v2		= new Text(i18n->TranslateString("Some players may have problems playing AAC\nfiles with ID3 tags attached. Please use this option only\nif you are sure that your player can handle these tags."), Point(text_note->GetUnscaledTextWidth() + 12, 38));

	group_id3v2->SetSize(Size(Math::Max(240, text_note->GetUnscaledTextWidth() + text_id3v2->GetUnscaledTextWidth() + 22), Math::Max(text_note->GetUnscaledTextHeight(), text_id3v2->GetUnscaledTextHeight()) + 48));

	group_id3v2->Add(check_id3v2);
	group_id3v2->Add(text_note);
	group_id3v2->Add(text_id3v2);

	group_mp4		= new GroupBox(i18n->TranslateString("File format"), Point(7, 11), Size(group_id3v2->GetWidth() / 2 - 4, 65));

	option_mp4		= new OptionBox("MP4", Point(10, 13), Size(group_mp4->GetWidth() - 20, 0), &fileFormat, 1);
	option_mp4->onAction.Connect(&ConfigureCoreAudio::SetFileFormat, this);

	option_aac		= new OptionBox("AAC", Point(10, 38), Size(group_mp4->GetWidth() - 20, 0), &fileFormat, 0);
	option_aac->onAction.Connect(&ConfigureCoreAudio::SetFileFormat, this);

	group_mp4->Add(option_mp4);
	group_mp4->Add(option_aac);

	group_extension		= new GroupBox(i18n->TranslateString("File extension"), Point(group_mp4->GetWidth() + 15 + (group_id3v2->GetWidth() % 2), 11), Size(group_id3v2->GetWidth() / 2 - 4, 65));

	option_extension_m4a	= new OptionBox(".m4a", Point(10, 13),					Size(group_extension->GetWidth() / 2 - 14, 0), &fileExtension, 0);
	option_extension_m4b	= new OptionBox(".m4b", Point(10, 38),					Size(group_extension->GetWidth() / 2 - 14, 0), &fileExtension, 1);
	option_extension_m4r	= new OptionBox(".m4r", Point(group_extension->GetWidth() / 2 + 4, 13), Size(group_extension->GetWidth() / 2 - 14, 0), &fileExtension, 2);
	option_extension_mp4	= new OptionBox(".mp4", Point(group_extension->GetWidth() / 2 + 4, 38), Size(group_extension->GetWidth() / 2 - 14, 0), &fileExtension, 3);

	group_extension->Add(option_extension_m4a);
	group_extension->Add(option_extension_m4b);
	group_extension->Add(option_extension_m4r);
	group_extension->Add(option_extension_mp4);

	i18n->SetContext("Encoders::AAC::Codec");

	layer_quality		= new Layer(i18n->TranslateString("Codec"));

	group_codec		= new GroupBox(i18n->TranslateString("Audio codec"), Point(7, 11), Size(group_id3v2->GetWidth(), 43));

	text_codec		= new Text(i18n->AddColon(i18n->TranslateString("Audio codec")), Point(10, 15));

	combo_codec		= new ComboBox(Point(text_codec->GetUnscaledTextSize().cx + 17, 12), Size(group_codec->GetWidth() - text_codec->GetUnscaledTextSize().cx - 27, 0));

	/* Query supported formats from Core Audio
	 * and add them to the combo box list.
	 */
	for (Int i = 0; i < 32 && formats.codecs[i] != 0; i++)
	{
		if	(formats.codecs[i] == 'aac ') combo_codec->AddEntry("MPEG4 AAC Low Complexity");
		else if	(formats.codecs[i] == 'aach') combo_codec->AddEntry("MPEG4 AAC High Efficiency");
		else if	(formats.codecs[i] == 'aacp') combo_codec->AddEntry("MPEG4 AAC High Efficiency v2");
		else if	(formats.codecs[i] == 'aacl') combo_codec->AddEntry("MPEG4 AAC Low Delay");
		else if	(formats.codecs[i] == 'aace') combo_codec->AddEntry("MPEG4 AAC Enhanced Low Delay");
		else if	(formats.codecs[i] == 'aacf') combo_codec->AddEntry("MPEG4 AAC Enhanced Low Delay SBR");
		else if	(formats.codecs[i] == 'aacs') combo_codec->AddEntry("MPEG4 AAC Spatial");
		else if (formats.codecs[i] == 'alac') combo_codec->AddEntry("Apple Lossless Audio Codec");
		else				      continue;

		codecs.Add(formats.codecs[i]);

		if ((UnsignedInt) config->GetIntValue("CoreAudio", "Codec", 'aac ') == formats.codecs[i]) combo_codec->SelectNthEntry(combo_codec->Length() - 1);
	}

	combo_codec->onSelectEntry.Connect(&ConfigureCoreAudio::SetCodec, this);

	group_codec->Add(text_codec);
	group_codec->Add(combo_codec);

	i18n->SetContext("Encoders::AAC::Quality");

	group_bitrate		= new GroupBox(i18n->TranslateString("Bitrate"), Point(7, 66), Size(group_id3v2->GetWidth(), 43));

	text_bitrate		= new Text(i18n->AddColon(i18n->TranslateString("Bitrate")), Point(10, 15));

	slider_bitrate		= new Slider(Point(text_bitrate->GetUnscaledTextSize().cx + 17, 13), Size(group_bitrate->GetWidth() - 97 - text_bitrate->GetUnscaledTextSize().cx, 0), OR_HORZ, &bitrate, 1, 2560);
	slider_bitrate->onValueChange.Connect(&ConfigureCoreAudio::SetBitrate, this);

	edit_bitrate		= new EditBox(String::FromInt(bitrate), Point(group_bitrate->GetWidth() - 72, 12), Size(31, 0), 3);
	edit_bitrate->SetFlags(EDB_NUMERIC);
	edit_bitrate->onInput.Connect(&ConfigureCoreAudio::SetBitrateByEditBox, this);

	text_bitrate_kbps	= new Text(i18n->TranslateString("%1 kbps", "Technical").Replace("%1", NIL).Replace(" ", NIL), Point(group_bitrate->GetWidth() - 34, 15));

	group_bitrate->Add(text_bitrate);
	group_bitrate->Add(slider_bitrate);
	group_bitrate->Add(edit_bitrate);
	group_bitrate->Add(text_bitrate_kbps);

	SetCodec();

	tabwidget->SetSize(group_id3v2->GetSize() + Size(18, 118));

	Add(tabwidget);

	tabwidget->Add(layer_quality);
	tabwidget->Add(layer_format);

	layer_format->Add(group_mp4);
	layer_format->Add(group_extension);
	layer_format->Add(group_id3v2);

	layer_quality->Add(group_codec);
	layer_quality->Add(group_bitrate);

	SetSize(group_id3v2->GetSize() + Size(32, 132));
}

BoCA::ConfigureCoreAudio::~ConfigureCoreAudio()
{
	DeleteObject(tabwidget);
	DeleteObject(layer_format);
	DeleteObject(layer_quality);

	DeleteObject(group_mp4);
	DeleteObject(option_mp4);
	DeleteObject(option_aac);
	DeleteObject(group_extension);
	DeleteObject(option_extension_m4a);
	DeleteObject(option_extension_m4b);
	DeleteObject(option_extension_m4r);
	DeleteObject(option_extension_mp4);
	DeleteObject(group_id3v2);
	DeleteObject(check_id3v2);
	DeleteObject(text_note);
	DeleteObject(text_id3v2);

	DeleteObject(group_codec);
	DeleteObject(text_codec);
	DeleteObject(combo_codec);
	DeleteObject(group_bitrate);
	DeleteObject(text_bitrate);
	DeleteObject(slider_bitrate);
	DeleteObject(edit_bitrate);
	DeleteObject(text_bitrate_kbps);
}

Int BoCA::ConfigureCoreAudio::SaveSettings()
{
	Config	*config = Config::Get();

	config->SetIntValue("CoreAudio", "Codec", codecs.GetNth(combo_codec->GetSelectedEntryNumber()));

	if (bitrates.Length() == 2)
	{
		if (bitrate < bitrates.GetNth(0)) bitrate = bitrates.GetNth(0);
		if (bitrate > bitrates.GetNth(1)) bitrate = bitrates.GetNth(1);
	}

	if	(bitrates.Length() == 2) config->SetIntValue("CoreAudio", "Bitrate", bitrate);
	else if (bitrates.Length() >  2) config->SetIntValue("CoreAudio", "Bitrate", bitrates.GetNth((bitrates.Length() / 2 + bitrate) * 2 + 1));

	config->SetIntValue("CoreAudio", "MP4Container", fileFormat);
	config->SetIntValue("CoreAudio", "MP4FileExtension", fileExtension);
	config->SetIntValue("CoreAudio", "AllowID3v2", allowID3);

	return Success();
}

Void BoCA::ConfigureCoreAudio::SetCodec()
{
	UnsignedInt	 format	= codecs.GetNth(combo_codec->GetSelectedEntryNumber());

	for (Int i = 0; i < 32 && formats.codecs[i] != 0; i++)
	{
		if (formats.codecs[i] != format) continue;

		Int	 numBitrates = 0;

		for (Int j = 0; j < 64; j++) if (formats.bitrates[i][2 * j + 1] != 0) numBitrates++;

		if (numBitrates > 0 && bitrates.Length() > 2) bitrate = bitrates.GetNth((bitrates.Length() / 2 + bitrate) * 2 + 1);

		bitrates.RemoveAll();

		for (Int j = 0; j < 64 && formats.bitrates[i][2 * j + 1] != 0; j++)
		{
			bitrates.Add(formats.bitrates[i][2 * j    ] / 1000);
			bitrates.Add(formats.bitrates[i][2 * j + 1] / 1000);

			if (bitrate == bitrates.GetNth(j * 2 + 1)) bitrate = -numBitrates + j;
		}

		break;
	}

	if (bitrates.Length() > 0) group_bitrate->Activate();
	else			   group_bitrate->Deactivate();

	if	(bitrates.Length() == 2) { edit_bitrate->Activate();   slider_bitrate->SetRange(bitrates.GetNth(0), bitrates.GetNth(1)); }
	else if (bitrates.Length() >  2) { edit_bitrate->Deactivate(); slider_bitrate->SetRange(-bitrates.Length() / 2, -1);		 }

	if	(codecs.GetNth(combo_codec->GetSelectedEntryNumber()) == 'aac ' ||
		 codecs.GetNth(combo_codec->GetSelectedEntryNumber()) == 'aach' ||
		 codecs.GetNth(combo_codec->GetSelectedEntryNumber()) == 'aacp' ||
		 codecs.GetNth(combo_codec->GetSelectedEntryNumber()) == 'aacl' ||
		 codecs.GetNth(combo_codec->GetSelectedEntryNumber()) == 'aace' ||
		 codecs.GetNth(combo_codec->GetSelectedEntryNumber()) == 'aacf' ||
		 codecs.GetNth(combo_codec->GetSelectedEntryNumber()) == 'aacs')   group_mp4->Activate();
	else if (codecs.GetNth(combo_codec->GetSelectedEntryNumber()) == 'alac') { group_mp4->Deactivate(); fileFormat = 1; }

	SetBitrate();
	SetFileFormat();
}

Void BoCA::ConfigureCoreAudio::SetBitrate()
{
	if (bitrates.Length() == 0) return;

	if (bitrates.Length() == 2) edit_bitrate->SetText(String::FromInt(bitrate));
	else			    edit_bitrate->SetText(String::FromInt(bitrates.GetNth((bitrates.Length() / 2 + bitrate) * 2 + 1)));
}

Void BoCA::ConfigureCoreAudio::SetBitrateByEditBox()
{
	slider_bitrate->SetValue(edit_bitrate->GetText().ToInt());
}

Void BoCA::ConfigureCoreAudio::SetFileFormat()
{
	if (fileFormat == 1)	// MP4 container
	{
		group_id3v2->Deactivate();

		group_extension->Activate();
	}
	else			// raw AAC file format
	{
		group_id3v2->Activate();

		group_extension->Deactivate();
	}
}