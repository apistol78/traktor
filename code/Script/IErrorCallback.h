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

namespace traktor::script
{

/*! Script error callback.
 * \ingroup Script
 */
class IErrorCallback
{
public:
	virtual ~IErrorCallback() {}

	/*! Syntax error callback. */
	virtual void syntaxError(const std::wstring& name, uint32_t line, const std::wstring& message) = 0;

	/*! Other error callback. */
	virtual void otherError(const std::wstring& message) = 0;
};

}
