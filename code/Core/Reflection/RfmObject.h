/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Reflection/ReflectionMember.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

/*! Reflected object member.
 * \ingroup Core
 */
class T_DLLCLASS RfmObject : public ReflectionMember
{
	T_RTTI_CLASS;

public:
	RfmObject(const wchar_t* name, ISerializable* value);

	void set(ISerializable* value) { m_value = value; }

	ISerializable* get() const { return m_value; }

	virtual bool replace(const ReflectionMember* source) override final;

private:
	Ref< ISerializable > m_value;
};

}

