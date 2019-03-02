#pragma once

#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace weather
	{

struct CloudParticle
{
	Vector4 position;
	float positionVelocity;
	float maxRadius;
	float radius;
	float rotation;
	float rotationVelocity;
	float opacity;
	int sprite;
};

	}
}

