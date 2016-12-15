#ifndef traktor_illuminate_Types_H
#define traktor_illuminate_Types_H

#include "Core/Ref.h"
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

	Surface()
	:	count(0)
	,	emissive(0.0f)
	,	translucency(0.0f)
	{
	}
};

struct Light
{
	int32_t type;		//<! 0 - directional, 1 - point
	Vector4 position;
	Vector4 direction;
	Color4f sunColor;
	Color4f baseColor;
	Color4f shadowColor;
	Scalar range;
	Ref< IProbe > probe;
	int32_t surface;

	Light()
	:	type(0)
	,	surface(0)
	{
	}
};

	}
}

#endif	// traktor_illuminate_Types_H

