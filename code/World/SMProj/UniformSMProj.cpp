#include "World/WorldRenderSettings.h"
#include "World/SMProj/UniformSMProj.h"

namespace traktor
{
	namespace world
	{

void calculateUniformSMProj(
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
	Vector4 viewDirection = viewInverse.axisZ();

	// Calculate light axises.
	Vector4 lightAxisX, lightAxisY, lightAxisZ;

	lightAxisZ = -lightDirection.normalized();
	lightAxisX = cross(lightAxisZ, -viewDirection).normalized();
	lightAxisY = cross(lightAxisX, lightAxisZ).normalized();

	// Adjust view frustum to shadowing far z.
	Frustum shadowViewFrustum = viewFrustum;
	shadowViewFrustum.setFarZ(Scalar(settings.shadowFarZ));

	// Calculate bounding box of view frustum in light space.
	Aabb3 viewFrustumBox;
	for (int i = 0; i < 8; ++i)
	{
		Vector4 worldCorner = viewInverse * shadowViewFrustum.corners[i];
		Vector4 lightCorner(
			dot3(lightAxisX, worldCorner),
			dot3(lightAxisY, worldCorner),
			dot3(lightAxisZ, worldCorner),
			1.0f
		);
		viewFrustumBox.contain(lightCorner);
	}

	// Update light view matrix with bounding box centered.
	Vector4 center = viewFrustumBox.getCenter();
	Vector4 extent = viewFrustumBox.getExtent() * Vector4(2.0f, 2.0f, 1.0f, 0.0f);

	// Calculate world center of view frustum's bounding box.
	Vector4 worldCenter =
		lightAxisX * center.x() +
		lightAxisY * center.y() +
		lightAxisZ * center.z() +
		Vector4::origo();

	Scalar lightDistance = Scalar(settings.depthRange);

	outLightView = Matrix44(
		lightAxisX,
		lightAxisY,
		lightAxisZ,
		worldCenter - lightAxisZ * lightDistance
	);

	outLightView = outLightView.inverseOrtho();

	outLightProjection = orthoLh(
		extent.x(),
		extent.y(),
		0.0f,
		lightDistance + extent.z()
	);

	outShadowFrustum.buildOrtho(
		extent.x(),
		extent.y(),
		0.0f,
		lightDistance + extent.z()
	);
}

	}
}
