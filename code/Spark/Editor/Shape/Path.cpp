#include "Spark/Editor/Shape/Path.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Path", Path, Object)

Path::Path()
:	m_origin(0.0f, 0.0f)
,	m_cursor(0.0f, 0.0f)
,	m_current(0)
{
}

void Path::moveTo(float x, float y, bool relative)
{
	if (relative)
		getAbsolute(x, y);

	m_origin.set(x, y);
	m_cursor.set(x, y);
	m_current = 0;
}

void Path::lineTo(float x, float y, bool relative)
{
	if (!m_current || m_current->type != SptLinear)
	{
		m_subPaths.push_back(SubPath(SptLinear, m_origin));
		m_current = &m_subPaths.back();
		m_current->points.push_back(m_cursor);
	}

	if (relative)
		getAbsolute(x, y);

	m_current->points.push_back(Vector2(x, y));
	m_cursor.set(x, y);
}

void Path::quadricTo(float x1, float y1, float x, float y, bool relative)
{
	if (!m_current || m_current->type != SptQuadric)
	{
		m_subPaths.push_back(SubPath(SptQuadric, m_origin));
		m_current = &m_subPaths.back();
		m_current->points.push_back(m_cursor);
	}

	if (relative)
	{
		getAbsolute(x1, y1);
		getAbsolute(x, y);
	}

	m_current->points.push_back(Vector2(x1, y1));
	m_current->points.push_back(Vector2(x , y ));
	m_cursor.set(x, y);
}

void Path::quadricTo(float x, float y, bool relative)
{
	float x1 = 0.0f;
	float y1 = 0.0f;
	quadricTo(x1, y1, x, y, relative);
}

void Path::cubicTo(float x1, float y1, float x2, float y2, float x, float y, bool relative)
{
	if (!m_current || m_current->type != SptCubic)
	{
		m_subPaths.push_back(SubPath(SptCubic, m_origin));
		m_current = &m_subPaths.back();
		m_current->points.push_back(m_cursor);
	}

	if (relative)
	{
		getAbsolute(x1, y1);
		getAbsolute(x2, y2);
		getAbsolute(x, y);
	}

	m_current->points.push_back(Vector2(x1, y1));
	m_current->points.push_back(Vector2(x2, y2));
	m_current->points.push_back(Vector2(x , y ));
	m_cursor.set(x, y);
}

void Path::cubicTo(float x2, float y2, float x, float y, bool relative)
{
	float x1 = (relative ? 0.0f : m_cursor.x) - (x2 - x);
	float y1 = (relative ? 0.0f : m_cursor.y) - (y2 - y);
	cubicTo(x1, y1, x2, y2, x, y, relative);
}

void Path::close()
{
	if (m_current)
	{
		m_cursor = m_current->origin;
		m_origin = m_current->origin;
		m_current->closed = true;
		m_current = 0;
	}
}

const Vector2& Path::getCursor() const
{
	return m_cursor;
}

const std::vector< SubPath >& Path::getSubPaths() const
{
	return m_subPaths;
}

void Path::getAbsolute(float& x, float& y) const
{
	x += m_cursor.x;
	y += m_cursor.y;
}

	}
}
