/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"

namespace traktor
{
	namespace run
	{

/*! Scriptable output writer interface.
 * \ingroup Run
 */
class IOutput : public Object
{
	T_RTTI_CLASS;

public:
	virtual void print(const std::wstring& s) = 0;

	virtual void printLn(const std::wstring& s) = 0;
};

	}
}

