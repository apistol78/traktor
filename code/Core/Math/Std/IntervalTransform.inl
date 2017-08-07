/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
namespace traktor
{

IntervalTransform::IntervalTransform()
{
}

IntervalTransform::IntervalTransform(const Transform& transform)
{
	m_transform[0] = transform;
	m_transform[1] = transform;
}

IntervalTransform::IntervalTransform(const Transform& transform0, const Transform& transform1)
{
	m_transform[0] = transform0;
	m_transform[1] = transform1;
}

void IntervalTransform::set(const Transform& transform)
{
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
	m_transform[0] = m_transform[1];
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