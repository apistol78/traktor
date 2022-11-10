/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Compress/Lzf/DeflateStreamLzf.h"
#	include "Compress/Lzf/InflateStreamLzf.h"
#	include "Compress/Lzo/DeflateStreamLzo.h"
#	include "Compress/Lzo/InflateStreamLzo.h"
#	include "Compress/Zip/DeflateStreamZip.h"
#	include "Compress/Zip/InflateStreamZip.h"

namespace traktor
{
	namespace compress
	{

extern "C" void __module__Traktor_Compress()
{
	T_FORCE_LINK_REF(DeflateStreamLzf);
	T_FORCE_LINK_REF(InflateStreamLzf);
	T_FORCE_LINK_REF(DeflateStreamLzo);
	T_FORCE_LINK_REF(InflateStreamLzo);
	T_FORCE_LINK_REF(DeflateStreamZip);
	T_FORCE_LINK_REF(InflateStreamZip);
}

	}
}

#endif
