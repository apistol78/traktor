/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Run/App/StreamInput.h"

namespace traktor::run
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
