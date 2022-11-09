#pragma once

#include "Core/Math/Vector4.h"

namespace traktor::world
{

enum
{
	MaxSliceCount = 4,
	MaxLightShadowCount = 2,
	MaxLightsPerCluster = 8,
	ClusterDimXY = 16,
	ClusterDimZ = 32
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

enum class LightType
{
	LtDisabled = 0,
	LtDirectional = 1,
	LtPoint = 2,
	LtSpot = 3
};

/*! Update parameters. */
struct UpdateParams
{
	Object* contextObject = nullptr;	/*!< Update context object; is Stage instance during runtime. */
	float totalTime = 0.0f;				/*!< Total time since first update. */
	float deltaTime = 0.0f;				/*!< Delta time since last update. */
	float alternateTime = 0.0f;			/*!< Alternative absolute time. */
};

}
