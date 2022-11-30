/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <limits>
#include "Core/Math/Aabb2.h"
#include "World/SMProj/TrapezoidShadowProjection.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.TrapezoidShadowProjection", TrapezoidShadowProjection, IShadowProjection)

void TrapezoidShadowProjection::calculate(
	const Matrix44& viewInverse,
	const Vector4& lightPosition,
	const Vector4& lightDirection,
	const Frustum& viewFrustum,
	float shadowFarZ,
	bool quantizeProjection,
	Matrix44& outLightView,
	Matrix44& outLightProjection,
	Frustum& outShadowFrustum
) const
{
	const Vector4 viewDirection = viewInverse.axisZ();

	// Calculate light axises.
	Vector4 lightAxisX, lightAxisY, lightAxisZ;

	lightAxisZ = -lightDirection.normalized();
	lightAxisX = -cross(lightAxisZ, -viewDirection).normalized();
	lightAxisY = cross(lightAxisX, lightAxisZ).normalized();

	// Calculate X/Y projection of view frustum in light space.
	AlignedVector< Vector2 > lightFrustumProj;
	Aabb3 viewFrustumBox;
	Aabb2 lightFrustumProjBox;
	for (int i = 0; i < 8; ++i)
	{
		const Vector4 worldCorner = viewInverse * viewFrustum.corners[i];
		const Vector4 lightCorner(
			dot3(lightAxisX, worldCorner),
			dot3(lightAxisY, worldCorner),
			dot3(lightAxisZ, worldCorner),
			1.0f
		);
		viewFrustumBox.contain(lightCorner);
		lightFrustumProj.push_back(Vector2(
			lightCorner.x(),
			lightCorner.y()
		));
		lightFrustumProjBox.contain(lightFrustumProj.back());
	}

	// Projection is already orientated properly; thus we can
	// quickly approximate best fitting, symmetrical, trapezoid.
	const float ey = lightFrustumProjBox.getExtent().y;
	float nw = 0.0f, fw = 0.0f;
	for (int i = 0; i < 8; ++i)
	{
		const Vector2 p = lightFrustumProj[i] - lightFrustumProjBox.getCenter();
		if (p.y < -0.8f * ey)
			nw = std::max(nw, abs(p.x) * 2.0f);
	}
	if (nw < 1.0f)
		nw = 1.0f;
	for (int i = 0; i < 8; ++i)
	{
		const Vector2 p = lightFrustumProj[i] - lightFrustumProjBox.getCenter();
		if (p.y >= -0.8f * ey)
		{
			const Vector2 p0(p.x < 0.0f ? -nw : nw, -ey);
			const Vector2 p1(p.x, p.y);
			const Vector2 d = (p1 - p0).normalized();
			const float w = p.x + d.x * (ey - p.y);
			fw = std::max(fw, abs(w) * 2.0f);
		}
	}

	const Matrix44 Mn(
		1.0f / nw, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	const float h = ey * 2.0f;
	const float k = fw / nw - 1.0f;
	const float s = (2.0f * k + 2.0f) / h;
	const Matrix44 Ms(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, s, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, k / h, 0.0f, 1.0f
	);

	const Matrix44 M0 = Ms * Mn;

	const Matrix44 T0(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, -1.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	//Matrix44 NT = T0 * M0;

	// Update light view matrix with bounding box centered.
	const Vector4 extent = viewFrustumBox.getExtent() * Scalar(2.0f);

	const float nz = viewFrustum.getNearZ();

	// Calculate light view and projection matrices.
	const Vector4 worldCenter = viewInverse * Vector4(0.0f, 0.0f, -nz, 1.0f);
	const Scalar lightDistance = Scalar(shadowFarZ * 2.0f);

	outLightView = Matrix44(
		lightAxisX,
		lightAxisY,
		lightAxisZ,
		worldCenter - lightAxisZ * lightDistance
	);

	outLightView = outLightView.inverse();

	outLightProjection = orthoLh(
		2.0f,
		2.0f,
		0.0f,
		lightDistance + extent.z()
	);

	// outLightSquareProjection = NT;

	outShadowFrustum.buildOrtho(
		extent.x(),
		extent.y() * 2.0f,	// \hack View transform is at near plane thus need to encompass all.
		0.0f,
		lightDistance + extent.z()
	);

	// Add part of view frustum to shadow frustum.
	const Matrix44 view2Light = outLightView * viewInverse;
	for (uint32_t i = 0; i < viewFrustum.planes.size(); ++i)
	{
		const Plane viewFrustumPlane = view2Light * viewFrustum.planes[i];
		if (viewFrustumPlane.normal().z() <= 0.0f)
			outShadowFrustum.planes.push_back(viewFrustumPlane);
	}
}

}
