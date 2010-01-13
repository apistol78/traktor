#include "World/BoxSMProj.h"
#include "World/WorldRenderSettings.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

Scalar scalarMin(const Scalar& a, const Scalar& b)
{
	return a < b ? a : b;
}

Scalar scalarMax(const Scalar& a, const Scalar& b)
{
	return a > b ? a : b;
}

		}

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
	const static Vector4 c_axisUp(0.0f, 1.0f, 0.0f);
	Scalar lightDistance(settings.viewFarZ);

	Vector4 lightAxisX, lightAxisY, lightAxisZ;
	lightAxisZ = -lightDirection.normalized();
	lightAxisX = cross(lightAxisZ, c_axisUp).normalized();
	lightAxisY = cross(lightAxisX, lightAxisZ).normalized();

	Vector4 shadowBoxExtents[8];
	shadowBox.getExtents(shadowBoxExtents);

	Scalar minX, maxX;
	Scalar minY, maxY;

	minX = maxX = dot3(lightAxisX, shadowBoxExtents[0]);
	minY = maxY = dot3(lightAxisY, shadowBoxExtents[0]);

	for (int i = 1; i < 8; ++i)
	{
		Scalar x = dot3(lightAxisX, shadowBoxExtents[i]);
		Scalar y = dot3(lightAxisY, shadowBoxExtents[i]);
		minX = scalarMin(x, minX);
		maxX = scalarMax(x, maxX);
		minY = scalarMin(y, minY);
		maxY = scalarMax(y, maxY);
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
