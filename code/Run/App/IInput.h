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

namespace traktor::run
{

/*! Scriptable input reader interface.
 * \ingroup Run
 */
class IInput : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool endOfFile() = 0;

	virtual std::wstring readChar() = 0;

	virtual std::wstring readLn() = 0;
};

}
