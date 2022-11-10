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

namespace traktor
{
	namespace render
	{

/*! Pin types.
 * \ingroup Render
 *
 * \note Ordered in predecence; higher number is higher predecence.
 */
enum class PinType
{
	Void = 0,
	Scalar1 = 1,
	Scalar2 = 2,
	Scalar3 = 3,
	Scalar4 = 4,
	Matrix = 5,
	Texture2D = 6,
	Texture3D = 7,
	TextureCube = 8,
	StructBuffer = 9,
	Image2D = 10,
	Image3D = 11,
	ImageCube = 12,
	State = 13,
	Any = (State)
};

/*! Return true if type is a scalar.
 * \ingroup Render
 */
inline bool isPinTypeScalar(PinType pinType)
{
	return pinType >= PinType::Scalar1 && pinType <= PinType::Scalar4;
}

/*! Return true if type is a texture.
 * \ingroup Render
 */
inline bool isPinTypeTexture(PinType pinType)
{
	return pinType >= PinType::Texture2D && pinType <= PinType::TextureCube;
}

/*! Return true if type is a struct buffer.
 * \ingroup Render
 */
inline bool isPinTypeStructBuffer(PinType pinType)
{
	return pinType == PinType::StructBuffer;
}

/*! Return true if type is a state.
 * \ingroup Render
 */
inline bool isPinTypeState(PinType pinType)
{
	return pinType == PinType::State;
}

/*! Get width of scalar type.
 * \ingroup Render
 */
inline int32_t getPinTypeWidth(PinType pinType)
{
	return isPinTypeScalar(pinType) ? int32_t(pinType) : 0;
}

/*! Pin order type.
 * \ingroup Render
 */
enum PinOrderType
{
	PotConstant = 0,	/*!< Constant order. */
	PotLinear = 1,		/*!< Linear order. */
	PotNonLinear = 2	/*!< Non-linear order, i.e. cubic or higher. */
};

inline PinOrderType pinOrderAdd(PinOrderType pinOrder1, PinOrderType pinOrder2)
{
	PinOrderType pinOrder = (PinOrderType)(pinOrder1 + pinOrder2);
	return pinOrder <= PotNonLinear ? pinOrder : PotNonLinear;
}

inline PinOrderType pinOrderMax(const PinOrderType* pinOrders, int32_t pinOrdersCount)
{
	PinOrderType pinOrder = PotConstant;
	for (int32_t i = 0; i < pinOrdersCount; ++i)
		pinOrder = (pinOrders[i] > pinOrder) ? pinOrders[i] : pinOrder;
	return pinOrder;
}

inline PinOrderType pinOrderMax(PinOrderType pinOrder1, PinOrderType pinOrder2)
{
	return pinOrder1 > pinOrder2 ? pinOrder1 : pinOrder2;
}

inline PinOrderType pinOrderConstantOrNonLinear(const PinOrderType* pinOrders, int32_t pinOrdersCount)
{
	for (int32_t i = 0; i < pinOrdersCount; ++i)
	{
		if (pinOrders[i] != PotConstant)
			return PotNonLinear;
	}
	return PotConstant;
}

	}
}

