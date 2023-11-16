/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <set>
#include "Core/Object.h"
#include "Core/Class/Any.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*!
 * \ingroup Core
 */
class T_DLLCLASS Preprocessor : public Object
{
	T_RTTI_CLASS;

public:
	void setDefinition(const std::wstring& symbol, const Any& value = Any());

	void removeDefinition(const std::wstring& symbol);

	bool evaluate(const std::wstring& source, std::wstring& output, std::set< std::wstring >& usings) const;

private:
	std::map< std::wstring, Any > m_definitions;

	int32_t evaluateExpression(const std::wstring& expression, const std::map< std::wstring, Any >& definitions) const;
};

}
