#ifndef traktor_render_PinType_H
#define traktor_render_PinType_H

#include "Core/Object.h"

namespace traktor
{
	namespace render
	{

/*! \brief Pin types.
 * \ingroup Render
 *
 * \note Ordered in predecence; higher number is higher predecence.
 */
enum PinType
{
	PntVoid		= 0,
	PntScalar1	= 1,
	PntScalar2	= 2,
	PntScalar3	= 3,
	PntScalar4	= 4,
	PntMatrix	= 5,
	PntTexture	= 6,

	PntAny		= (PntTexture)
};

/*! \brief Return true if type is a scalar.
 * \ingroup Render
 */
inline bool isPinTypeScalar(PinType pinType)
{
	return pinType >= PntScalar1 && pinType <= PntScalar4;
}

/*! \brief Get width of scalar type.
 * \ingroup Render
 */
inline int32_t getPinTypeWidth(PinType pinType)
{
	return isPinTypeScalar(pinType) ? int32_t(pinType) : 0;
}

	}
}

#endif	// traktor_render_PinType_H
