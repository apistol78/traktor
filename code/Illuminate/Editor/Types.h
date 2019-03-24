#ifndef traktor_illuminate_Types_H
#define traktor_illuminate_Types_H

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace illuminate
	{

class IProbe;

struct Surface
{
	int32_t count;
	Vector4 points[16];
	Vector2 texCoords[16];
	Vector4 normals[16];
	Vector4 normal;
	Color4f color;
	Scalar emissive;
	Scalar translucency;
	AlignedVector< uint32_t > shared[16];

	Surface()
	:	count(0)
	,	emissive(0.0f)
	,	translucency(0.0f)
	{
	}
};

struct Light
{
	enum LightType
	{
		LtDirectional = 0,
		LtPoint = 1,
		LtSpot = 2,
		LtProbe = 3
	};

	LightType type;
	Vector4 position;
	Vector4 direction;
	Color4f color;
	Scalar range;
	Scalar radius;
	Ref< IProbe > probe;
	int32_t surface;

	Light()
	:	type(LtDirectional)
	,	position(Vector4::origo())
	,	direction(Vector4::zero())
	,	color(0.0f, 0.0f, 0.0f, 1.0f)
	,	range(0.0f)
	,	radius(0.0f)
	,	surface(0)
	{
	}
};

	}
}

#endif	// traktor_illuminate_Types_H

