/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
