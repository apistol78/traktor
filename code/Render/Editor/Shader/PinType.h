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
enum PinType
{
	PntVoid = 0,
	PntScalar1 = 1,
	PntScalar2 = 2,
	PntScalar3 = 3,
	PntScalar4 = 4,
	PntMatrix = 5,
	PntTexture2D = 6,
	PntTexture3D = 7,
	PntTextureCube = 8,
	PntStructBuffer = 9,
	PntImage2D = 10,
	PntImage3D = 11,
	PntImageCube = 12,
	PntState = 13,
	PntAny = (PntState)
};

/*! Return true if type is a scalar.
 * \ingroup Render
 */
inline bool isPinTypeScalar(PinType pinType)
{
	return pinType >= PntScalar1 && pinType <= PntScalar4;
}

/*! Return true if type is a texture.
 * \ingroup Render
 */
inline bool isPinTypeTexture(PinType pinType)
{
	return pinType >= PntTexture2D && pinType <= PntTextureCube;
}

/*! Return true if type is a struct buffer.
 * \ingroup Render
 */
inline bool isPinTypeStructBuffer(PinType pinType)
{
	return pinType == PntStructBuffer;
}

/*! Return true if type is a state.
 * \ingroup Render
 */
inline bool isPinTypeState(PinType pinType)
{
	return pinType == PntState;
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

