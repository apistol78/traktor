/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
}

void StdOutput::printLn(const std::wstring& s)
{
#if !defined(__APPLE__)
	fwprintf(m_handle, L"%ls\n", s.c_str());
#else
	fprintf(m_handle, "%s\n", wstombs(s).c_str());
#endif
}

	}
}
