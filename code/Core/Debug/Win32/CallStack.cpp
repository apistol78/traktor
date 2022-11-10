/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <Windows.h>
#include <DbgHelp.h>
#include "Core/Debug/CallStack.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace
	{

static bool s_initialized = false;

	}

uint32_t getCallStack(uint32_t ncs, void** outCs, uint32_t skip)
{
	return CaptureStackBackTrace(
		skip + 1,
		ncs,
		outCs,
		0
	);
}

bool getSymbolFromAddress(const void* address, std::wstring& outSymbol)
{
	if (!s_initialized)
	{
		SymInitialize(
			GetCurrentProcess(),
			NULL,
			TRUE
		);
		s_initialized = true;
	}

	uint8_t buf[sizeof(SYMBOL_INFO) + 1024];

	SYMBOL_INFO& si = *(SYMBOL_INFO*)buf;
	si.SizeOfStruct = sizeof(SYMBOL_INFO);
	si.MaxNameLen = 1024;

	if (!SymFromAddr(
		GetCurrentProcess(),
		(DWORD64)address,
		0,
		&si
	))
		return false;

	outSymbol = mbstows(si.Name);
	return true;
}

bool getSourceFromAddress(const void* address, std::wstring& outSource, int32_t& outLine)
{
	if (!s_initialized)
	{
		SymInitialize(
			GetCurrentProcess(),
			NULL,
			TRUE
		);
		s_initialized = true;
	}

	IMAGEHLP_LINE64 ln = {};
	ln.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	char fileName[MAX_PATH + 1];
	ln.FileName = fileName;

	DWORD displacement = 0;
	if (!SymGetLineFromAddr(
		GetCurrentProcess(),
		(DWORD64)address,
		&displacement,
		&ln
	))
		return false;

	outSource = mbstows(ln.FileName);
	outLine = (int32_t)ln.LineNumber;
	return true;
}

}
