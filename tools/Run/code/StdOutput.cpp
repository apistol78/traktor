#include "StdOutput.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"StdOutput", StdOutput, IOutput)

StdOutput::StdOutput(FILE* handle)
:	m_handle(handle)
{
}

void StdOutput::print(const std::wstring& s)
{
	fwprintf(m_handle, s.c_str());
}

void StdOutput::printLn(const std::wstring& s)
{
	fwprintf(m_handle, L"%s\n", s.c_str());
}
