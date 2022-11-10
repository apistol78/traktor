/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Net/Replication/State/IValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class T_DLLCLASS FloatValue : public RefCountImpl< IValue >
{
	T_RTTI_CLASS;

public:
	typedef float value_t;
	typedef float value_argument_t;
	typedef float value_return_t;

	FloatValue()
	:	m_value(0.0f)
	{
	}

	explicit FloatValue(float value)
	:	m_value(value)
	{
	}

	operator float () const { return m_value; }

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	float m_value;
};

	}
}

