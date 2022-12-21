/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Rotator/WobbleComponent.h"
#include "Core/Math/Const.h"
#include "World/Entity.h"

namespace traktor::animation
{
	namespace
	{

float convolve(float x, const float* k, int32_t nk)
{
	float o = 0.0f;
	for (int32_t i = 0; i < nk; i += 3)
	{
		const float* lk = &k[i];
		o += sinf(x * lk[0] + lk[1]) * lk[2];
	}
	return o;
}

const float c_wobbleX[] =
{
	4.0f, 0.0f, 0.2f,
	5.0f, 0.4f, 0.1f,
	7.0f, 0.6f, 0.05f
};

const float c_wobbleY[] =
{
	3.0f, 0.4f, 0.2f,
	6.0f, 0.7f, 0.1f,
	8.0f, 1.1f, 0.05f
};

const float c_wobbleZ[] =
{
	2.0f, 0.8f, 0.2f,
	5.0f, 1.2f, 0.1f,
	8.0f, 1.6f, 0.05f
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.WobbleComponent", WobbleComponent, world::IEntityComponent)

WobbleComponent::WobbleComponent(float magnitude, float rate)
:	m_magnitude(magnitude)
,	m_rate(rate)
{
}

void WobbleComponent::destroy()
{
	m_owner = nullptr;
}

void WobbleComponent::setOwner(world::Entity* owner)
{
	if ((m_owner = owner) != nullptr)
		m_transform = m_owner->getTransform();
}

void WobbleComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 WobbleComponent::getBoundingBox() const
{
	return Aabb3();
}

void WobbleComponent::update(const world::UpdateParams& update)
{
	if (!m_owner)
		return;

	if (update.deltaTime <= FUZZY_EPSILON)
	{
		m_owner->setTransform(m_transform);
		return;
	}
	
	if (m_magnitude > FUZZY_EPSILON && m_rate > FUZZY_EPSILON)
	{
		const float dx = convolve(update.totalTime * m_rate, c_wobbleX, sizeof_array(c_wobbleX)) * m_magnitude;
		const float dy = convolve(update.totalTime * m_rate, c_wobbleY, sizeof_array(c_wobbleY)) * m_magnitude;
		const float dz = convolve(update.totalTime * m_rate, c_wobbleZ, sizeof_array(c_wobbleZ)) * m_magnitude;
		m_local = Transform(Vector4(dx, dy, dz));
	}

	m_owner->setTransform(m_transform * m_local);
}

}
