#include "Core/Math/IntervalTransform.h"

namespace traktor
{

IntervalTransform::IntervalTransform()
:	m_step(false)
{
}

IntervalTransform::IntervalTransform(const Transform& transform)
:	m_step(false)
{
	m_transform[0] = transform;
	m_transform[1] = transform;
}

void IntervalTransform::set(const Transform& transform)
{
	if (m_step)
	{
		m_transform[0] = m_transform[1];
		m_step = false;
	}
	m_transform[1] = transform;
}

Transform IntervalTransform::get(float interval) const
{
	return lerp(m_transform[0], m_transform[1], Scalar(interval));
}

void IntervalTransform::step()
{
	if (m_step)
		m_transform[0] = m_transform[1];
	else
		m_step = true;
}

}
