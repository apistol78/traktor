/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ActionTypes_H
#define traktor_flash_ActionTypes_H

#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

// Verbose logging.
#if defined(_DEBUG)
#	define T_IF_VERBOSE(x) { x }
#else
#	define T_IF_VERBOSE(x)
#endif

	}
}

#endif	// traktor_flash_ActionTypes_H
