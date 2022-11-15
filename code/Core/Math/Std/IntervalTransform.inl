/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
namespace traktor
{

IntervalTransform::IntervalTransform(const Transform& transform)
:	m_stepped(true)
{
	m_transform[0] = transform;
	m_transform[1] = transform;
}

IntervalTransform::IntervalTransform(const Transform& transform0, const Transform& transform1)
:	m_stepped(false)
{
	m_transform[0] = transform0;
	m_transform[1] = transform1;
}

void IntervalTransform::set(const Transform& transform)
{
	if (m_stepped)
	{
		m_transform[0] = m_transform[1];
		m_stepped = false;
	}
	m_transform[1] = transform;
}

Transform IntervalTransform::get(float interval) const
{
	return lerp(
		m_transform[0],
		m_transform[1],
		Scalar(interval)
	);
}

void IntervalTransform::step()
{
	if (m_stepped)
		m_transform[0] = m_transform[1];

	m_stepped = true;
}

const Transform& IntervalTransform::get0() const
{
	return m_transform[0];
}

const Transform& IntervalTransform::get() const
{
	return m_transform[1];
}

}
