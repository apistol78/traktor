/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Run/App/StreamOutput.h"

namespace traktor::run
{
	namespace
	{

OutputStream::LineEnd translateLineEnding(const std::wstring& lineEnding)
{
	if (lineEnding == L"auto")
		return OutputStream::LineEnd::Auto;
	else if (lineEnding == L"win")
		return OutputStream::LineEnd::Win;
	else if (lineEnding == L"mac")
		return OutputStream::LineEnd::Mac;
	else if (lineEnding == L"unix")
		return OutputStream::LineEnd::Unix;
	else
		return OutputStream::LineEnd::Auto;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.run.StreamOutput", StreamOutput, IOutput)

StreamOutput::StreamOutput(IStream* stream, IEncoding* encoding)
:	m_output(stream, encoding)
{
}

StreamOutput::StreamOutput(IStream* stream, IEncoding* encoding, const std::wstring& lineEnding)
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

}
