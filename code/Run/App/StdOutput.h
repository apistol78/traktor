/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cstdio>
#include "Run/App/IOutput.h"

namespace traktor
{
	namespace run
	{

/*! Standard IO output writer.
 * \ingroup Run
 */
class StdOutput : public IOutput
{
	T_RTTI_CLASS;

public:
	StdOutput(FILE* handle);

	virtual void print(const std::wstring& s) override final;

	virtual void printLn(const std::wstring& s) override final;

private:
	FILE* m_handle;
};

	}
}

