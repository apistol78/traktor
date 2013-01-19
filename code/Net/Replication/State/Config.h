#ifndef traktor_net_Config_H
#define traktor_net_Config_H

#include "Core/Math/Scalar.h"

namespace traktor
{
	namespace net
	{

const Scalar c_maxRubberBandDistance(200.0f);
const float c_maxRubberBandTime = 0.5f;
const float c_rubberBandStrengthNear = 0.99f;
const float c_rubberBandStrengthFar = 0.85f;

	}
}

#endif	// traktor_net_Config_H
