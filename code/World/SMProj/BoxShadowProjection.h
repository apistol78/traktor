#ifndef traktor_world_BoxShadowProjection_H
#define traktor_world_BoxShadowProjection_H

#include "World/IWorldShadowProjection.h"

namespace traktor
{
	namespace world
	{

class BoxShadowProjection : public IWorldShadowProjection
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
		Matrix44& outLightSquareProjection,
		Frustum& outShadowFrustum
	) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_world_BoxShadowProjection_H
