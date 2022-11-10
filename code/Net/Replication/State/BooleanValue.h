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

class T_DLLCLASS BooleanValue : public RefCountImpl< IValue >
{
	T_RTTI_CLASS;

public:
	typedef bool value_t;
	typedef bool value_argument_t;
	typedef bool value_return_t;

	BooleanValue()
	:	m_value(false)
	{
	}

	explicit BooleanValue(bool value)
	:	m_value(value)
	{
	}

	operator bool () const { return m_value; }

	void* operator new (size_t size);

	void operator delete (void* ptr);

private:
	bool m_value;
};

	}
}

