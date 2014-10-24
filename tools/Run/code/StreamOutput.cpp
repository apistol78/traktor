#include "StreamOutput.h"

using namespace traktor;

namespace
{

	OutputStream::LineEnd translateLineEnding(const std::wstring& lineEnding)
	{
		if (lineEnding == L"auto")
			return OutputStream::LeAuto;
		else if (lineEnding == L"win")
			return OutputStream::LeWin;
		else if (lineEnding == L"mac")
			return OutputStream::LeMac;
		else if (lineEnding == L"unix")
			return OutputStream::LeUnix;
		else
			return OutputStream::LeAuto;
	}

}

T_IMPLEMENT_RTTI_CLASS(L"StreamOutput", StreamOutput, IOutput)

StreamOutput::StreamOutput(IStream* stream, IEncoding* encoding)
:	m_output(stream, encoding)
{
}

StreamOutput::StreamOutput(IStream* stream, IEncoding* encoding, std::wstring& lineEnding)
:	m_output(stream, encoding, translateLineEnding(lineEnding))
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
