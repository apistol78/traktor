#include "Core/Math/IntervalTransform.h"

namespace traktor
{

IntervalTransform::IntervalTransform()
:	m_modified(false)
{
}

IntervalTransform::IntervalTransform(const Transform& transform)
:	m_modified(false)
{
	m_transform[0] = transform;
	m_transform[1] = transform;
}

void IntervalTransform::set(const Transform& transform)
{
	m_transform[1] = transform;
	m_modified = true;
}

Transform IntervalTransform::get(float interval) const
{
	return lerp(
		m_transform[0],
		m_transform[1],
		Scalar(clamp(interval, 0.0f, 1.0f))
	);
}

void IntervalTransform::step()
{
	if (m_modified)
	{
		m_transform[0] = m_transform[1];
		m_modified = false;
	}
}

}
