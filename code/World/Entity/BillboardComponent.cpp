/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/BillboardComponent.h"

#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Matrix44.h"
#include "Render/Shader.h"

#include <cmath>

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.BillboardComponent", BillboardComponent, IEntityComponent)

BillboardComponent::BillboardComponent(
	const resource::Proxy< render::Shader >& shader,
	int32_t angles,
	int32_t elevations,
	float elevationFrom,
	float elevationTo,
	float size,
	const Vector4& offset,
	float startDistance,
	float cullDistance)
	: m_shader(shader)
	, m_size(size)
	, m_offset(offset)
	, m_startDistance(startDistance)
	, m_cullDistance(cullDistance)
{
	angles = max< int32_t >(angles, 1);
	elevations = max< int32_t >(elevations, 1);

	m_views.reserve(angles * elevations);

	for (int32_t ey = 0; ey < elevations; ++ey)
	{
		const float elevation = (elevations > 1) ? elevationFrom + (elevationTo - elevationFrom) * ey / (elevations - 1) : elevationFrom;
		for (int32_t ex = 0; ex < angles; ++ex)
		{
			const float yaw = TWO_PI * ex / angles;

			// Same rotation the atlas was rasterized with, thus its inverse take the axes
			// of that view back into the entity's own space.
			const Matrix44 orientation = (rotateX(-elevation) * rotateY(yaw)).inverse();

			m_views.push_back(View{
				orientation * Vector4(1.0f, 0.0f, 0.0f, 0.0f),
				orientation * Vector4(0.0f, 1.0f, 0.0f, 0.0f),
				orientation * Vector4(0.0f, 0.0f, 1.0f, 0.0f),
				Vector4(
					(float)ex / angles,
					(float)ey / elevations,
					(float)(ex + 1) / angles,
					(float)(ey + 1) / elevations) });
		}
	}
}

void BillboardComponent::destroy()
{
	m_shader.clear();
	m_views.clear();
}

void BillboardComponent::setOwner(Entity* owner)
{
}

void BillboardComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 BillboardComponent::getBoundingBox() const
{
	// The quad swing around its centre as the camera move, so the box which hold it in
	// every orientation is the sphere it sweep, not the quad itself.
	const float radius = m_size * 0.5f * std::sqrt(2.0f);
	const Vector4 extent(radius, radius, radius, 0.0f);
	return Aabb3(m_offset.xyz1() - extent, m_offset.xyz1() + extent);
}

void BillboardComponent::update(const UpdateParams& update)
{
}

const BillboardComponent::View& BillboardComponent::findView(const Vector4& direction) const
{
	T_ASSERT(!m_views.empty());

	const Vector4 d = direction.xyz0().normalized();

	const View* best = &m_views.front();
	Scalar bestDot = dot3(best->forward, d);

	for (uint32_t i = 1; i < m_views.size(); ++i)
	{
		const Scalar k = dot3(m_views[i].forward, d);
		if (k > bestDot)
		{
			bestDot = k;
			best = &m_views[i];
		}
	}

	return *best;
}

}
