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

namespace traktor::render
{

/*! Pin types.
 * \ingroup Render
 *
 * \note Ordered in predecence; higher number is higher predecence.
 */
enum class PinType : int32_t
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
	Bundle = 14,
	Any = (Bundle)
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

/*! Return true if type is an image.
 * \ingroup Render
 */
inline bool isPinTypeImage(PinType pinType)
{
	return pinType >= PinType::Image2D && pinType <= PinType::ImageCube;
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
enum class PinOrder : int32_t
{
	Constant = 0,	/*!< Constant order. */
	Linear = 1,		/*!< Linear order. */
	NonLinear = 2	/*!< Non-linear order, i.e. cubic or higher. */
};

inline PinOrder pinOrderAdd(PinOrder pinOrder1, PinOrder pinOrder2)
{
	PinOrder pinOrder = (PinOrder)((int32_t)pinOrder1 + (int32_t)pinOrder2);
	return pinOrder <= PinOrder::NonLinear ? pinOrder : PinOrder::NonLinear;
}

inline PinOrder pinOrderMax(const PinOrder* pinOrders, int32_t pinOrdersCount)
{
	PinOrder pinOrder = PinOrder::Constant;
	for (int32_t i = 0; i < pinOrdersCount; ++i)
		pinOrder = (pinOrders[i] > pinOrder) ? pinOrders[i] : pinOrder;
	return pinOrder;
}

inline PinOrder pinOrderMax(PinOrder pinOrder1, PinOrder pinOrder2)
{
	return pinOrder1 > pinOrder2 ? pinOrder1 : pinOrder2;
}

inline PinOrder pinOrderConstantOrNonLinear(const PinOrder* pinOrders, int32_t pinOrdersCount)
{
	for (int32_t i = 0; i < pinOrdersCount; ++i)
	{
		if (pinOrders[i] != PinOrder::Constant)
			return PinOrder::NonLinear;
	}
	return PinOrder::Constant;
}

}
