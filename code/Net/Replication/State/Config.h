#ifndef traktor_net_Config_H
#define traktor_net_Config_H

#include "Core/Math/Scalar.h"

namespace traktor
{
	namespace net
	{

const Scalar c_maxRubberBandDistance(120.0f);
const float c_maxRubberBandTime = 0.2f;
const float c_rubberBandStrengthNear = 0.95f;
const float c_rubberBandStrengthFar = 0.8f;

	}
}

#endif	// traktor_net_Config_H
