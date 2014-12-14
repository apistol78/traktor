#ifndef traktor_terrain_IFallOff_H
#define traktor_terrain_IFallOff_H

#include "Core/Object.h"

namespace traktor
{
	namespace terrain
	{

/*! \brief Brush falloff.
 * \ingroup Terrain
 */
class IFallOff : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Evaluate brush falloff.
	 *
	 * \param x Brush X coordinate, -1 to 1.
	 * \param y Brush Y coordinate, -1 to 1.
	 * \return Falloff value, 0 no brush influence, 1 full brush influence.
	 */
	virtual float evaluate(float x, float y) const = 0;
};

	}
}

#endif	// traktor_terrain_IFallOff_H
