/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#if defined(__LINUX__) || defined(__APPLE__)
#	include <execinfo.h>
#endif
#include "Core/Platform.h"
#include "Core/Debug/CallStack.h"

namespace traktor
{

uint32_t getCallStack(uint32_t ncs, void** outCs, uint32_t skip)
{
#if defined(__LINUX__) || defined(__APPLE__)
	return backtrace(
		outCs,
		ncs
	);
#else
	return 0;
#endif
}

}
