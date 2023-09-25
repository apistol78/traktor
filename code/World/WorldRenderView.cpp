/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "World/WorldRenderView.h"

/*! Enable screen-space culling; quite expensive but might yield less draw calls. */
#define T_ENABLE_SCREENSPACE_CULLING 1

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderView", WorldRenderView, Object)

WorldRenderView::WorldRenderView()
:	m_index(0)
,	m_snapshot(false)
,	m_projection(Matrix44::identity())
,	m_lastView(Matrix44::identity())
,	m_view(Matrix44::identity())
,	m_viewSize(0.0f, 0.0f)
,	m_time(0.0f)
,	m_deltaTime(0.0f)
,	m_interval(0.0f)
{
}

void WorldRenderView::setIndex(int32_t index)
{
	m_index = index;
}

void WorldRenderView::setSnapshot(bool snapshot)
{
	m_snapshot = snapshot;
}

void WorldRenderView::setOrthogonal(float width, float height, float nearZ, float farZ)
{
	Frustum viewFrustum;
	viewFrustum.buildOrtho(width, height, -farZ, farZ);

	setViewSize(Vector2(width, height));
	setViewFrustum(viewFrustum);
	setCullFrustum(viewFrustum);
	setProjection(orthoLh(width, height, -farZ, farZ));
}

void WorldRenderView::setPerspective(float width, float height, float aspect, float fov, float nearZ, float farZ)
{
	Frustum viewFrustum;
	viewFrustum.buildPerspective(fov, aspect, nearZ, farZ);

	setViewSize(Vector2(width, height));
	setViewFrustum(viewFrustum);
	setCullFrustum(viewFrustum);
	setProjection(perspectiveLh(fov, aspect, nearZ, farZ));
}

void WorldRenderView::setViewFrustum(const Frustum& viewFrustum)
{
	m_viewFrustum = viewFrustum;
}

void WorldRenderView::setCullFrustum(const Frustum& cullFrustum)
{
	m_cullFrustum = cullFrustum;
}

void WorldRenderView::setProjection(const Matrix44& projection)
{
	m_projection = projection;
}

void WorldRenderView::setView(const Matrix44& lastView, const Matrix44& view)
{
	m_lastView = lastView;
	m_view = view;

	// Calculate eye position and direction in world space.
	const Matrix44 viewInverse = view.inverse();
	m_eyePosition = viewInverse.translation().xyz1();
	m_eyeDirection = viewInverse.axisZ().xyz0();
}

void WorldRenderView::setViewSize(const Vector2& viewSize)
{
	m_viewSize = viewSize;
}

void WorldRenderView::setTimes(double time, double deltaTime, float interval)
{
	m_time = time;
	m_deltaTime = deltaTime;
	m_interval = interval;
}

bool WorldRenderView::isBoxVisible(const Aabb3& box, const Transform& worldTransform, const float minScreenArea, float& outDistance) const
{
	// Earliest, empty bounding boxes cannot contain anything visible.
	if (box.empty())
		return false;

	const Matrix44 worldView = m_view * worldTransform.toMatrix44();

	// Early out of bounding sphere is outside of frustum.
	const Vector4 center = worldView * box.getCenter();
	const Scalar radius = box.getExtent().length();

	if (m_cullFrustum.inside(center, radius) == Frustum::Result::Outside)
		return false;

#if T_ENABLE_SCREENSPACE_CULLING

	if (minScreenArea > FUZZY_EPSILON)
	{
		// Project bounding box extents onto view plane.
		Vector4 extents[8];
		box.getExtents(extents);

		Vector4 mn(
			std::numeric_limits< float >::max(),
			std::numeric_limits< float >::max(),
			std::numeric_limits< float >::max(),
			std::numeric_limits< float >::max()
		);
		Vector4 mx(
			-std::numeric_limits< float >::max(),
			-std::numeric_limits< float >::max(),
			-std::numeric_limits< float >::max(),
			-std::numeric_limits< float >::max()
		);

		const Matrix44 worldViewProj = m_projection * worldView;

		for (int i = 0; i < sizeof_array(extents); ++i)
		{
			Vector4 p = worldViewProj * extents[i];
			if (p.w() <= 0.0_simd)
			{
				// Bounding box clipped to view plane; assume it's visible.
				outDistance = center.z() + radius;
				return true;
			}

			// Homogeneous divide.
			p /= p.w();

			// Track screen space extents.
			mn = min(mn, p);
			mx = max(mx, p);
		}

		// Ensure we're visible.
		if (mn.x() > 1.0_simd || mn.y() > 1.0_simd || mx.x() < -1.0_simd || mx.y() < -1.0_simd)
			return false;

		// Calculate screen area, cull if it's below threshold.
		const Vector4 e = mx - mn;
		if (max(e.x(), e.y()) < minScreenArea)
			return false;
	}

#endif

	outDistance = center.z() + radius;
	return true;
}

}
