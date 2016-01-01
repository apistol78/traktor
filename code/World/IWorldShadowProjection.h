#ifndef traktor_world_IWorldShadowProjection_H
#define traktor_world_IWorldShadowProjection_H

#include "Core/Object.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"

namespace traktor
{
	namespace world
	{

class IWorldShadowProjection : public Object
{
	T_RTTI_CLASS;

public:
	virtual void calculate(
		const Matrix44& viewInverse,
		const Vector4& lightPosition,
		const Vector4& lightDirection,
		const Frustum& viewFrustum,
		const Aabb3& shadowBox,
		float shadowFarZ,
		bool quantizeProjection,
		Matrix44& outLightView,
		Matrix44& outLightProjection,
		Frustum& outShadowFrustum
	) const = 0;
};

	}
}

#endif	// traktor_world_IWorldShadowProjection_H
