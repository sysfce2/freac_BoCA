 /* The smooth Class Library
  * Copyright (C) 1998-2008 Robert Kausch <robert.kausch@gmx.net>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of "The Artistic License, Version 2.0".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#ifndef _H_OBJSMOOTH_TIPODAYDLG_
#define _H_OBJSMOOTH_TIPODAYDLG_

namespace smooth
{
	namespace GUI
	{
		namespace Dialogs
		{
			class TipOfTheDay;
		};

		class Window;
		class Button;
		class CheckBox;
		class Text;
		class Titlebar;
		class Divider;
		class Layer;
		class Image;
	};
};

#include "dialog.h"

namespace smooth
{
	namespace GUI
	{
		namespace Dialogs
		{
			class SMOOTHAPI TipOfTheDay : public Dialog
			{
				private:
					Window		*dlgwnd;
					Titlebar	*titlebar;
					Divider		*divbar;
					Button		*btn_ok;
					Button		*btn_next;
					CheckBox	*check_showtips;
					Image		*img_light;
					Text		*txt_didyouknow;
					Text		*txt_tip;
					Layer		*layer_inner;

					Bool		*showTips;

					Array<String>	 tips;
					Int		 mode;
					Int		 offset;
				public:
							 TipOfTheDay(Bool *);
					virtual		~TipOfTheDay();

					const Error	&ShowDialog();

					Int		 AddTip(const String &);
					Int		 SetMode(Int, Int = 0, Bool = True);

					Int		 GetOffset();
				slots:
					Void		 ButtonOK();
					Void		 ButtonNext();

					Void		 Paint();
			};

			const Int	 TIP_ORDERED	= 0;
			const Int	 TIP_RANDOM	= 1;
		};
	};
};

#endif