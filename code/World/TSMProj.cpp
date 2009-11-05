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
	Vector4 frustumWorld[8];
	Vector2 frustumLight[8];
	
	// Transform view frustum corners into world space.
	for (int i = 0; i < 8; ++i)
		frustumWorld[i] = viewInverse * viewFrustum.corners[i];

	// View direction in world space.
	Vector4 viewDirection = viewInverse.axisZ();

	Vector4 lightAxisU = cross(lightDirection, viewDirection).normalized();
	Vector4 lightAxisV = cross(lightAxisU, lightDirection).normalized();

	// Project world corners onto light axises; 2d set of frustum.
	for (int i = 0; i < 8; ++i)
	{
		frustumLight[i].x = dot3(lightAxisU, frustumWorld[i]);
		frustumLight[i].y = dot3(lightAxisV, frustumWorld[i]);
	}




}

	}
}
