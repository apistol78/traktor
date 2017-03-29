namespace traktor
{

IntervalTransform::IntervalTransform()
:	m_stepped(false)
{
}

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
		Scalar(clamp(interval, 0.0f, 1.0f))
	);
}

void IntervalTransform::step()
{
	if (m_stepped)
		m_transform[0] = m_transform[1];

	m_stepped = true;
}

const Transform& IntervalTransform::get() const
{
	return m_transform[1];
}

Transform IntervalTransform::getDelta() const
{
	return m_transform[1] * m_transform[0].inverse();
}

}