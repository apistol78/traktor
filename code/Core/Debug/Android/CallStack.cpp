/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#if defined(__LINUX__) || defined(__RPI__) || defined(__APPLE__)
#	include <execinfo.h>
#endif
#include "Core/Platform.h"
#include "Core/Debug/CallStack.h"

namespace traktor
{

uint32_t getCallStack(uint32_t ncs, void** outCs, uint32_t skip)
{
#if defined(__LINUX__) || defined(__RPI__) || defined(__APPLE__)
	return backtrace(
		outCs,
		ncs
	);
#else
	return 0;
#endif
}

bool getSymbolFromAddress(const void* address, std::wstring& outSymbol)
{
	return false;
}

bool getSourceFromAddress(const void* address, std::wstring& outSource, int32_t& outLine)
{
	return false;
}

}
