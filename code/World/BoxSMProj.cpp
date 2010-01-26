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
	const Aabb& shadowBox,
	Matrix44& outLightView,
	Matrix44& outLightProjection,
	Frustum& outShadowFrustum
)
{
	Scalar lightDistance(settings.viewFarZ);

	Vector4 shadowBoxExtents[8];
	shadowBox.getExtents(shadowBoxExtents);

	Vector4 shadowBoxEdge = (shadowBoxExtents[6] - shadowBoxExtents[0]).normalized();

	Vector4 lightAxisX, lightAxisY, lightAxisZ;
	lightAxisZ = -lightDirection.normalized();
	lightAxisX = cross(shadowBoxEdge, lightAxisZ).normalized();
	lightAxisY = cross(lightAxisX, lightAxisZ).normalized();

	Scalar minX, maxX;
	Scalar minY, maxY;

	minX = maxX = dot3(lightAxisX, shadowBoxExtents[0]);
	minY = maxY = dot3(lightAxisY, shadowBoxExtents[0]);

	for (int i = 1; i < 8; ++i)
	{
		Scalar x = dot3(lightAxisX, shadowBoxExtents[i]);
		Scalar y = dot3(lightAxisY, shadowBoxExtents[i]);
		minX = min(x, minX);
		maxX = max(x, maxX);
		minY = min(y, minY);
		maxY = max(y, maxY);
	}

	outLightView = Matrix44(
		lightAxisX,
		lightAxisY,
		lightAxisZ,
		shadowBox.getCenter().xyz1()
	);

	outLightView = outLightView.inverseOrtho();

	outLightProjection = orthoLh(
		maxX - minX,
		maxY - minY,
		-lightDistance * Scalar(0.5f),
		lightDistance * Scalar(0.5f)
	);

	outShadowFrustum.buildOrtho(
		maxX - minX,
		maxY - minY,
		-lightDistance * Scalar(0.5f),
		lightDistance * Scalar(0.5f)
	);
}

	}
}
