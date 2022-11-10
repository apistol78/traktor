/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/StringReader.h"
#include "Run/App/IInput.h"

namespace traktor
{
	namespace run
	{

/*! Stream input reader.
 * \ingroup Run
 */
class StreamInput : public IInput
{
	T_RTTI_CLASS;

public:
	StreamInput(IStream* stream, IEncoding* encoding);

	virtual bool endOfFile() override final;

	virtual std::wstring readChar() override final;

	virtual std::wstring readLn() override final;

private:
	Ref< IStream > m_stream;
	StringReader m_reader;
};

	}
}

