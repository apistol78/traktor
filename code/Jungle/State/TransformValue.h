/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Transform.h"
#include "Jungle/State/IValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_JUNGLE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::jungle
{

class T_DLLCLASS TransformValue : public RefCountImpl< IValue >
{
	T_RTTI_CLASS;

public:
	typedef Transform value_t;
	typedef const Transform& value_argument_t;
	typedef Transform value_return_t;

	TransformValue() = default;

	explicit TransformValue(const Transform& value)
	:	m_value(value)
	{
	}

	operator const Transform& () const { return m_value; }

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	Transform m_value;
};

}
