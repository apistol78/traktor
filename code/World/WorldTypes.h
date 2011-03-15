#ifndef traktor_world_WorldTypes_H
#define traktor_world_WorldTypes_H

#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace world
	{

enum
{
	MaxSliceCount = 4,
	MaxLightCount = 16
};

enum LightType
{
	LtDisabled = 0,
	LtDirectional = 1,
	LtPoint = 2
};

struct Light
{
	LightType type;
	Vector4 position;
	Vector4 direction;
	Vector4 sunColor;
	Vector4 baseColor;
	Vector4 shadowColor;
	Scalar range;
};

	}
}

#endif	// traktor_world_WorldTypes_H
