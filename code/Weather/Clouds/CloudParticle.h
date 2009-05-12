#ifndef traktor_weather_CloudParticle_H
#define traktor_weather_CloudParticle_H

#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace weather
	{

struct CloudParticle
{
	Vector4 position;
	float radius;
	float rotation;
	int sprite;
};

	}
}

#endif	// traktor_weather_CloudParticle_H
