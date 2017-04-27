/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Sound/WinMM/SoundDriverWinMM.h"

namespace traktor
{
	namespace sound
	{

extern "C" void __module__Traktor_Sound_WinMM()
{
	T_FORCE_LINK_REF(SoundDriverWinMM);
}

	}
}

#endif
