/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

#endif	// traktor_weather_CloudParticle_H
