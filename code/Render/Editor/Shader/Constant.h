/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/Editor/Shader/PinType.h"

namespace traktor
{
	namespace render
	{

/*! Evaluation SIMD constant.
 * \ingroup Render
 *
 * Primarily used when evaluating shader graphs during
 * constant folding optimization pass.
 */
class Constant
{
public:
	Constant();

	explicit Constant(PinType type);

	explicit Constant(float x);

	explicit Constant(float x, float y, float z, float w);

	/*! Cast constant into desired type.
	 *
	 * \param type Cast to type.
	 * \return Constant of type.
	 */
	Constant cast(PinType type) const;

	/*! Get type of constant. */
	PinType getType() const;

	/*! Get width of scalar constant. */
	int32_t getWidth() const;

	/*! Set channel as variant. */
	void setVariant(int32_t index);

	/*! Set constant value into channel. */
	void setValue(int32_t index, float value);

	/*! Get constant value from channel. */
	float getValue(int32_t index) const;

	/*! Check if all channels is constant. */
	bool isAllConst() const;

	/*! Check if any channel is constant. */
	bool isAnyConst() const;

	/*! Check if value in channel is constant. */
	bool isConst(int32_t index) const;

	/*! Check if all channels constain zero. */
	bool isAllZero() const;

	/*! Check if channel is zero. */
	bool isZero(int32_t index) const;

	/*! Check if all channels constain one. */
	bool isAllOne() const;

	/*! Check if channel is one. */
	bool isOne(int32_t index) const;

	bool operator == (const Constant& rh) const;

	bool operator != (const Constant& rh) const;

	bool isConstX() const { return isConst(0); }

	bool isConstY() const { return isConst(1); }

	bool isConstZ() const { return isConst(2); }

	bool isConstW() const { return isConst(3); }

	float x() const { return getValue(0); }

	float y() const { return getValue(1); }

	float z() const { return getValue(2); }

	float w() const { return getValue(3); }

private:
	PinType m_type;
	bool m_const[4];
	float m_value[4];
};

	}
}

