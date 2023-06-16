/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

 // import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Unit type
 * \ingroup UI
 * 
 * Values using unit type are used for device independent (dpi etc)
 * positions, sizes etc.
 * 
 * A unit size is defined being 1/96 of an inch.
 * 
 * Having a special type help guide user to use correct
 * values when setting up user interface which
 * are properly DPI aware.
 */
class T_DLLCLASS Unit
{
public:
	Unit() = default;

	explicit Unit(int32_t value)
	:	m_value(value)
	{
	}

	int32_t get() const { return m_value; }

	Unit operator - () const { return Unit(-m_value); }

	Unit operator + (const Unit& rh) const { return Unit(m_value + rh.m_value); }

	Unit operator - (const Unit& rh) const { return Unit(m_value - rh.m_value); }

	Unit operator * (const Unit& rh) const { return Unit(m_value * rh.m_value); }

	Unit operator / (const Unit& rh) const { return Unit(m_value / rh.m_value); }

	bool operator > (int32_t value) const { return m_value > value; }

	bool operator < (int32_t value) const { return m_value < value; }

private:
	int32_t m_value;
};

}

inline T_DLLCLASS traktor::ui::Unit operator ""_ut(unsigned long long value)
{
	return traktor::ui::Unit((int32_t)value);
}
