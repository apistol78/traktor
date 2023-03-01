/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Envelope key.
 * \ingroup UI
 */
class T_DLLCLASS EnvelopeKey : public Object
{
	T_RTTI_CLASS;

public:
	explicit EnvelopeKey(float T, float value, bool fixedT = false, bool fixedValue = false);

	void setT(float T) { m_T = T; }

	float getT() const { return m_T; }

	void setValue(float value) { m_value = value; }

	float getValue() const { return m_value; }

	bool isFixedT() const { return m_fixedT; }

	bool isFixedValue() const { return m_fixedValue; }

private:
	float m_T;
	float m_value;
	bool m_fixedT;
	bool m_fixedValue;
};

}
