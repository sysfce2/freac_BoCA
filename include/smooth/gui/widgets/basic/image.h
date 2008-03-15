 /* The smooth Class Library
  * Copyright (C) 1998-2008 Robert Kausch <robert.kausch@gmx.net>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of "The Artistic License, Version 2.0".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#ifndef _H_OBJSMOOTH_IMAGE_
#define _H_OBJSMOOTH_IMAGE_

namespace smooth
{
	namespace GUI
	{
		class Image;
	};
};

#include "../widget.h"
#include "../../../graphics/bitmap.h"

namespace smooth
{
	namespace GUI
	{
		class SMOOTHAPI Image : public Widget
		{
			protected:
				Bitmap			 bitmap;
			public:
				static const Int	 classID;

							 Image(const Bitmap &, const Point & = Point(0, 0), const Size & = Size(0, 0));
				virtual			~Image();

				virtual Int		 Paint(Int);
			accessors:
				Int			 SetBitmap(const Bitmap &);
				const Bitmap		&GetBitmap();
		};
	};
};

#endif