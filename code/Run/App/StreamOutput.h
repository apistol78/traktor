/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/FileOutputStream.h"
#include "Run/App/IOutput.h"

namespace traktor::run
{

/*! Stream output writer.
 * \ingroup Run
 */
class StreamOutput : public IOutput
{
	T_RTTI_CLASS;

public:
	explicit StreamOutput(IStream* stream, IEncoding* encoding);

	explicit StreamOutput(IStream* stream, IEncoding* encoding, const std::wstring& lineEnding);

	virtual void print(const std::wstring& s) override final;

	virtual void printLn(const std::wstring& s) override final;

private:
	FileOutputStream m_output;
};

}
