#include "World/LiSPSMProj.h"
#include "World/WorldRenderSettings.h"
#include "Core/Math/Vector2.h"

namespace traktor
{
	namespace world
	{

void calculateTSMProj(
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
	outLightView = Matrix44::identity();
	outLightProjection = Matrix44::identity();
}

	}
}
