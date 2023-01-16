/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Script/IValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::script
{

/*!
 * \ingroup Script
 */
class T_DLLCLASS Value : public IValue
{
	T_RTTI_CLASS;

public:
	Value() = default;

	Value(const std::wstring& literal);

	void setLiteral(const std::wstring& literal);

	const std::wstring& getLiteral() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_literal;
};

}
