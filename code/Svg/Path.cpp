/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Svg/Path.h"

namespace traktor::svg
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.Path", Path, Object)

Path::Path()
:	m_origin(0.0f, 0.0f)
,	m_cursor(0.0f, 0.0f)
,	m_current(nullptr)
{
}

void Path::moveTo(float x, float y, bool relative)
{
	if (relative)
		getAbsolute(x, y);

	m_origin.set(x, y);
	m_cursor.set(x, y);
	m_current = nullptr;
}

void Path::lineTo(float x, float y, bool relative)
{
	if (!m_current || m_current->type != SubPathType::Linear)
	{
		m_subPaths.push_back(SubPath(SubPathType::Linear, m_origin));
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
	if (!m_current || m_current->type != SubPathType::Quadric)
	{
		m_subPaths.push_back(SubPath(SubPathType::Quadric, m_origin));
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
	const float x1 = 0.0f;
	const float y1 = 0.0f;
	quadricTo(x1, y1, x, y, relative);
}

void Path::cubicTo(float x1, float y1, float x2, float y2, float x, float y, bool relative)
{
	if (!m_current || m_current->type != SubPathType::Cubic)
	{
		m_subPaths.push_back(SubPath(SubPathType::Cubic, m_origin));
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
	const float x1 = (relative ? 0.0f : m_cursor.x) - (x2 - x);
	const float y1 = (relative ? 0.0f : m_cursor.y) - (y2 - y);
	cubicTo(x1, y1, x2, y2, x, y, relative);
}

void Path::close()
{
	if (m_current)
	{
		m_cursor = m_current->origin;
		m_origin = m_current->origin;
		m_current->closed = true;
		m_current = nullptr;
	}
}

void Path::transform(const Matrix33& transform)
{
	for (auto& subPath : m_subPaths)
	{
		subPath.origin = transform * subPath.origin;
		for (auto& point : subPath.points)
			point = transform * point;
	}
	m_origin = transform * m_origin;
	m_cursor = transform * m_cursor;
}

const Vector2& Path::getCursor() const
{
	return m_cursor;
}

const AlignedVector< SubPath >& Path::getSubPaths() const
{
	return m_subPaths;
}

void Path::getAbsolute(float& x, float& y) const
{
	x += m_cursor.x;
	y += m_cursor.y;
}

float Path::getAbsoluteX(float x) const
{
	return x + m_cursor.x;
}

float Path::getAbsoluteY(float y) const
{
	return y + m_cursor.y;
}

}
