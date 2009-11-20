#include "Flash/Path.h"
#include "Core/Log/Log.h"
namespace traktor
{
	namespace flash
	{

Path::Path()
:	m_cursor(0.0f, 0.0f)
{
}

void Path::reset()
{
	//m_cursor.x =
	//m_cursor.y = 0;
	m_subPaths.resize(0);
	m_current.segments.resize(0);
}

void Path::moveTo(float x, float y, CoordinateMode mode)
{
	T_ASSERT (m_current.segments.empty());
	transform(mode, CmAbsolute, x, y);
	m_cursor = Vector2(x, y);
}

void Path::lineTo(float x, float y, CoordinateMode mode)
{
	transform(mode, CmAbsolute, x, y);

	m_current.segments.push_back(SubPathSegment(SpgtLinear));
	m_current.segments.back().points.push_back(m_cursor);
	m_current.segments.back().points.push_back(Vector2(x, y));

	m_cursor = Vector2(x, y);
}

void Path::quadraticTo(float x1, float y1, float x, float y, CoordinateMode mode)
{
	transform(mode, CmAbsolute, x1, y1);
	transform(mode, CmAbsolute, x, y);

	m_current.segments.push_back(SubPathSegment(SpgtQuadratic));
	m_current.segments.back().points.push_back(m_cursor);
	m_current.segments.back().points.push_back(Vector2(x1, y1));
	m_current.segments.back().points.push_back(Vector2(x, y));

	m_cursor = Vector2(x, y);
}

void Path::end(uint16_t fillStyle0, uint16_t fillStyle1, uint16_t lineStyle)
{
	m_current.fillStyle0 = fillStyle0;
	m_current.fillStyle1 = fillStyle1;
	m_current.lineStyle = lineStyle;

	if (!m_current.segments.empty())
	{
		m_subPaths.push_back(m_current);
		m_current.segments.resize(0);
	}
}

Vector2 Path::getCursor() const
{
	return m_cursor;
}

const std::list< SubPath >& Path::getSubPaths() const
{
	return m_subPaths;
}

void Path::transform(CoordinateMode from, CoordinateMode to, float& x, float& y) const
{
	if (from == to)
		return;

	switch (from)
	{
	case CmRelative:
		x += m_cursor.x;
		y += m_cursor.y;
		break;

	case CmAbsolute:
		x -= m_cursor.x;
		y -= m_cursor.y;
		break;
	}
}

	}
}
