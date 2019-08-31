#pragma once

#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace world
	{

enum
{
	MaxSliceCount = 4,
	MaxLightShadowCount = 2
};

enum CancelType
{
	CtImmediate = 0,
	CtEnd = 1
};

enum Quality
{
	QuDisabled = 0,
	QuLow = 1,
	QuMedium = 2,
	QuHigh = 3,
	QuUltra = 4,
	QuLast = 5
};

enum CameraType
{
	CtOrthographic = 0,
	CtPerspective = 1
};

enum LightType
{
	LtDisabled = 0,
	LtDirectional = 1,
	LtPoint = 2,
	LtSpot = 3,
	LtProbe = 4	/*!< \deprecated */
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

/*! \brief Update parameters. */
struct UpdateParams
{
	float totalTime;		/*!< Total time since first update. */
	float deltaTime;		/*!< Delta time since last update. */
	float alternateTime;	/*!< Alternative absolute time. */
};

	}
}

