/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/TString.h"
#include "Run/App/StdOutput.h"

namespace traktor
{
	namespace run
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.run.StdOutput", StdOutput, IOutput)

StdOutput::StdOutput(FILE* handle)
:	m_handle(handle)
{
}

void StdOutput::print(const std::wstring& s)
{
#if !defined(__APPLE__)
	fwprintf(m_handle, L"%ls", s.c_str());
#else
	fprintf(m_handle, "%s", wstombs(s).c_str());
#endif
	fflush(m_handle);
}

void StdOutput::printLn(const std::wstring& s)
{
#if !defined(__APPLE__)
	fwprintf(m_handle, L"%ls\n", s.c_str());
#else
	fprintf(m_handle, "%s\n", wstombs(s).c_str());
#endif
	fflush(m_handle);
}

	}
}
