#include <Core/Misc/TString.h>
#include "StdOutput.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"StdOutput", StdOutput, IOutput)

StdOutput::StdOutput(FILE* handle)
:	m_handle(handle)
{
}

void StdOutput::print(const std::wstring& s)
{
#if !defined(__APPLE__)
	fwprintf(m_handle, L"%s", s.c_str());
#else
	fprintf(m_handle, "%s", wstombs(s).c_str());
#endif
}

void StdOutput::printLn(const std::wstring& s)
{
#if !defined(__APPLE__)
	fwprintf(m_handle, L"%s\n", s.c_str());
#else
	fprintf(m_handle, "%s\n", wstombs(s).c_str());
#endif
}
