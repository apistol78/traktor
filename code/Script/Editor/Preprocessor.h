/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <set>
#include "Core/Object.h"

namespace traktor::script
{

/*!
 * \ingroup Script
 */
class Preprocessor : public Object
{
	T_RTTI_CLASS;

public:
	void setDefinition(const std::wstring& symbol, int32_t value = 0);

	void removeDefinition(const std::wstring& symbol);

	bool evaluate(const std::wstring& source, std::wstring& output, std::set< std::wstring >& usings) const;

private:
	std::map< std::wstring, int32_t > m_definitions;

	int32_t evaluateExpression(const std::wstring& expression) const;
};

}
