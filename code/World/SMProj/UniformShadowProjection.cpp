#include "World/SMProj/UniformShadowProjection.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.UniformShadowProjection", UniformShadowProjection, IWorldShadowProjection)

UniformShadowProjection::UniformShadowProjection(
	const WorldRenderSettings& settings,
	uint32_t realShadowMapSize
)
:	m_settings(settings)
,	m_realShadowMapSize(float(realShadowMapSize))
{
}

void UniformShadowProjection::calculate(
	const Matrix44& viewInverse,
	const Vector4& lightPosition,
	const Vector4& lightDirection,
	const Frustum& viewFrustum,
	const Aabb3& shadowBox,
	Matrix44& outLightView,
	Matrix44& outLightProjection,
	Matrix44& outLightSquareProjection,
	Frustum& outShadowFrustum
) const
{
	// Calculate light axises.
	Vector4 lightAxisX, lightAxisY, lightAxisZ;

	lightAxisZ = -lightDirection.normalized();
	lightAxisX = cross(lightAxisZ, Vector4(0.0f, 1.0f, 0.0f, 0.0f)).normalized();
	lightAxisY = cross(lightAxisX, lightAxisZ).normalized();

	// Calculate bounding box of view frustum in light space.
	Aabb3 viewFrustumBox;
	for (int i = 0; i < 8; ++i)
	{
		Vector4 worldCorner = viewInverse * viewFrustum.corners[i];
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

	const float c_extentStep = 8.0f;

	Scalar ex = Scalar(std::ceil(extent.x() / c_extentStep) * c_extentStep);
	Scalar ey = Scalar(std::ceil(extent.y() / c_extentStep) * c_extentStep);

	Scalar smx = ex / Scalar(m_realShadowMapSize);
	Scalar smy = ey / Scalar(m_realShadowMapSize);

	Scalar cx = Scalar(std::floor(center.x() / smx) * smx);
	Scalar cy = Scalar(std::floor(center.y() / smy) * smy);

	// Calculate world center of view frustum's bounding box.
	Vector4 worldCenter =
		lightAxisX * cx +
		lightAxisY * cy +
		lightAxisZ * center.z() +
		Vector4::origo();

	Scalar lightDistance = Scalar(m_settings.depthRange);

	outLightView = Matrix44(
		lightAxisX,
		lightAxisY,
		lightAxisZ,
		worldCenter - lightAxisZ * lightDistance
	);

	outLightView = outLightView.inverseOrtho();

	outLightProjection = orthoLh(
		ex,
		ey,
		0.0f,
		lightDistance + extent.z()
	);

	outShadowFrustum.buildOrtho(
		ex,
		ey,
		0.0f,
		lightDistance + extent.z()
	);

	// Add part of view frustum to shadow frustum.
	Matrix44 view2Light = outLightView * viewInverse;
	for (uint32_t i = 0; i < viewFrustum.planes.size(); ++i)
	{
		Plane viewFrustumPlane = view2Light * viewFrustum.planes[i];
		if (viewFrustumPlane.normal().z() <= 0.0f)
			outShadowFrustum.planes.push_back(viewFrustumPlane);
	}

	outLightSquareProjection = Matrix44::identity();
}

	}
}
