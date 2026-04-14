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
#if defined(__LINUX__) || defined(__RPI__)
#	include <cstdio>
#	include <cstdlib>
#	include <cstring>
#	include <dlfcn.h>
#	include <cxxabi.h>
#endif
#include "Core/Debug/CallStack.h"
#if defined(__LINUX__) || defined(__RPI__)
#	include "Core/Misc/TString.h"
#endif

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
#if defined(__LINUX__) || defined(__RPI__)
	Dl_info info;
	if (!dladdr(address, &info) || !info.dli_sname)
		return false;

	int status = 0;
	char* demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
	if (demangled)
	{
		outSymbol = mbstows(demangled);
		free(demangled);
	}
	else
		outSymbol = mbstows(info.dli_sname);

	return true;
#else
	return false;
#endif
}

bool getSourceFromAddress(const void* address, std::wstring& outSource, int32_t& outLine)
{
#if defined(__LINUX__) || defined(__RPI__)
	Dl_info info;
	if (!dladdr(address, &info) || !info.dli_fname)
		return false;

	// Compute the address to pass to addr2line; for position-independent
	// executables / shared libraries we need the offset relative to the
	// object's load address.
	const uintptr_t offset = (uintptr_t)address - (uintptr_t)info.dli_fbase;

	char cmd[512];
	snprintf(cmd, sizeof(cmd), "addr2line -e %s -C -p 0x%lx 2>/dev/null",
		info.dli_fname, (unsigned long)offset);

	FILE* fp = popen(cmd, "r");
	if (!fp)
		return false;

	char buf[1024];
	if (!fgets(buf, sizeof(buf), fp))
	{
		pclose(fp);
		return false;
	}
	pclose(fp);

	// addr2line output with -p: "function at source:line" or "?? at ??:0"
	const char* at = strstr(buf, " at ");
	const char* src = at ? at + 4 : buf;

	// Strip trailing newline.
	char* nl = strchr(buf, '\n');
	if (nl)
		*nl = '\0';

	// Find the colon separating file:line.
	const char* colon = strrchr(src, ':');
	if (!colon || colon == src)
		return false;

	std::string file(src, colon - src);
	int line = atoi(colon + 1);

	// addr2line returns "??:0" when it can't resolve.
	if (file == "??" || line == 0)
		return false;

	outSource = mbstows(file);
	outLine = (int32_t)line;
	return true;
#else
	return false;
#endif
}

}
