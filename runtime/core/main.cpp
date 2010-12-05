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

#ifndef __WIN32__
#  include <stdlib.h>
#endif

#include <boca/application/registry.h>

#include <boca/common/config.h>
#include <boca/common/i18n.h>

#include <boca/common/communication/joblist.h>
#include <boca/common/communication/menu.h>

Void smooth::AttachDLL(Void *instance)
{
	BoCA::Config	*config = BoCA::Config::Get();

	/* Set number of threads for OpenMP optimized encoders.
	 */
	Int	 numThreads = config->GetIntValue("OpenMP", "NumberOfThreads", 0);

	if (numThreads == 0) numThreads = CPU().GetNumCores();

#ifdef __WIN32__
	if (GetEnvironmentVariableA("OMP_NUM_THREADS", NIL, 0) == 0) SetEnvironmentVariableA("OMP_NUM_THREADS", String::FromInt(numThreads));
#else
	if (getenv("OMP_NUM_THREADS") == NIL) setenv("OMP_NUM_THREADS", String::FromInt(numThreads), True);
#endif
}

Void smooth::DetachDLL()
{
	BoCA::AS::Registry::Free();

	BoCA::JobList::Free();
	BoCA::Menu::Free();

	BoCA::I18n::Free();
	BoCA::Config::Free();
}