/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "World/SMProj/BoxShadowProjection.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.BoxShadowProjection", BoxShadowProjection, IWorldShadowProjection)

void BoxShadowProjection::calculate(
	const Matrix44& viewInverse,
	const Vector4& lightPosition,
	const Vector4& lightDirection,
	const Frustum& viewFrustum,
	const Aabb3& shadowBox,
	float shadowFarZ,
	bool quantizeProjection,
	Matrix44& outLightView,
	Matrix44& outLightProjection,
	Frustum& outShadowFrustum
) const
{
	Scalar lightDistance(shadowFarZ * 2.0f);

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

	outLightView = outLightView.inverse();

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

	// Add part of view frustum to shadow frustum.
	Matrix44 view2Light = outLightView * viewInverse;
	for (uint32_t i = 0; i < viewFrustum.planes.size(); ++i)
	{
		Plane viewFrustumPlane = view2Light * viewFrustum.planes[i];
		if (viewFrustumPlane.normal().z() <= 0.0f)
			outShadowFrustum.planes.push_back(viewFrustumPlane);
	}
}

	}
}
