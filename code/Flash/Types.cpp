/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Thread/Atomic.h"
#include "Flash/Types.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

int32_t s_tag = 0;

		}

int32_t allocateCacheTag()
{
	return Atomic::increment(s_tag);
}

	}
}
