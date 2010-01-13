#include "World/BoxSMProj.h"
#include "World/WorldRenderSettings.h"

namespace traktor
{
	namespace world
	{

void calculateBoxSMProj(
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
	const static Vector4 c_axisUp(0.0f, 1.0f, 0.0f);
	Scalar shadowBoxExtent(settings.shadowFarZ);
	Scalar lightDistance(settings.viewFarZ);

	Vector4 lightAxisX, lightAxisY, lightAxisZ;
	lightAxisZ = -lightDirection.normalized();
	lightAxisX = cross(lightAxisZ, c_axisUp).normalized();
	lightAxisY = cross(lightAxisX, lightAxisZ).normalized();

	outLightView = Matrix44(
		lightAxisX,
		lightAxisY,
		lightAxisZ,
		Vector4::origo()
	);

	outLightView = outLightView.inverseOrtho();

	outLightProjection = orthoLh(
		shadowBoxExtent * Scalar(2.0f),
		shadowBoxExtent * Scalar(2.0f),
		-lightDistance * Scalar(0.5f),
		lightDistance * Scalar(0.5f)
	);

	outShadowFrustum.buildOrtho(
		shadowBoxExtent * Scalar(2.0f),
		shadowBoxExtent * Scalar(2.0f),
		-lightDistance * Scalar(0.5f),
		lightDistance * Scalar(0.5f)
	);
}

	}
}
