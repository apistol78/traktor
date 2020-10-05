#pragma once

#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace world
	{

enum
{
	MaxSliceCount = 4,
	MaxLightShadowCount = 2,
	ClusterDimXY = 32,
	ClusterDimZ = 4
};

enum class Cancel
{
	Immediate = 0,
	End = 1
};

enum class Quality
{
	Disabled = 0,
	Low = 1,
	Medium = 2,
	High = 3,
	Ultra = 4,
	Last = 5
};

enum class Projection
{
	Orthographic = 0,
	Perspective = 1
};

enum LightType
{
	LtDisabled = 0,
	LtDirectional = 1,
	LtPoint = 2,
	LtSpot = 3
};

struct Light
{
	LightType type;
	Vector4 position;
	Vector4 direction;
	Vector4 color;
	Scalar range;
	Scalar radius;
	bool castShadow;
};

/*! Update parameters. */
struct UpdateParams
{
	float totalTime;		/*!< Total time since first update. */
	float deltaTime;		/*!< Delta time since last update. */
	float alternateTime;	/*!< Alternative absolute time. */
};

	}
}

