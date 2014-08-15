#include "StreamOutput.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"StreamOutput", StreamOutput, IOutput)

StreamOutput::StreamOutput(IStream* stream, IEncoding* encoding)
:	m_output(stream, encoding)
{
}

void StreamOutput::print(const std::wstring& s)
{
	m_output << s;
}

void StreamOutput::printLn(const std::wstring& s)
{
	m_output << s << Endl;
}
