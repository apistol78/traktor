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

namespace traktor
{
	namespace script
	{

class Variable;

/*!
 * \ingroup Script
 */
class T_DLLCLASS ValueObject : public IValue
{
	T_RTTI_CLASS;

public:
	ValueObject() = default;

	explicit ValueObject(uint32_t objectRef);

	explicit ValueObject(uint32_t objectRef, const std::wstring& valueOf);

	void setObjectRef(uint32_t objectRef);

	uint32_t getObjectRef() const;

	void setValueOf(const std::wstring& valueOf);

	const std::wstring& getValueOf() const;

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_objectRef = 0;
	std::wstring m_valueOf;
};

	}
}

