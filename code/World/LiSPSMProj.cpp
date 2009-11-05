#include "World/LiSPSMProj.h"
#include "World/WorldRenderSettings.h"

namespace traktor
{
	namespace world
	{

void calculateLiSPSMProj(
	const WorldRenderSettings& settings,
	const Matrix44& viewInverse,
	const Vector4& lightPosition,
	const Vector4& lightDirection,
	const Frustum& viewFrustum,
	Matrix44& outLightView,
	Matrix44& outLightProjection,
	Frustum& outShadowFrustum
)
{
}

	}
}
