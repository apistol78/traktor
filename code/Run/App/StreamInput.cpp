/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/IStream.h"
#include "Run/App/StreamInput.h"

namespace traktor
{
	namespace run
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.run.StreamInput", StreamInput, IInput)

StreamInput::StreamInput(IStream* stream, IEncoding* encoding)
:	m_stream(stream)
,	m_reader(stream, encoding)
{
}

bool StreamInput::endOfFile()
{
	return m_stream->available() <= 0;
}

std::wstring StreamInput::readChar()
{
	wchar_t cs[] = { m_reader.readChar(), 0 };
	return cs;
}

std::wstring StreamInput::readLn()
{
	std::wstring line;
	if (m_reader.readLine(line) >= 0)
		return line;
	else
		return L"";
}

	}
}
