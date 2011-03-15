#ifndef traktor_world_BoxSMProj_H
#define traktor_world_BoxSMProj_H

#include "Core/Math/Matrix44.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Frustum.h"

namespace traktor
{
	namespace world
	{

class WorldRenderSettings;

void calculateBoxSMProj(
	const WorldRenderSettings& settings,
	const Matrix44& viewInverse,
	const Vector4& lightPosition,
	const Vector4& lightDirection,
	const Frustum& viewFrustum,
	const Aabb3& shadowBox,
	Matrix44& outLightView,
	Matrix44& outLightProjection,
	Matrix44& outLightSquareProjection,
	Frustum& outShadowFrustum
);

	}
}

#endif	// traktor_world_BoxSMProj_H
