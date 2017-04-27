/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include "Core/Io/StringOutputStream.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/Common/Point.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Point", Point, ActionObjectRelay)

Point::Point()
:	ActionObjectRelay("flash.geom.Point")
,	m_x(0.0f)
,	m_y(0.0f)
{
}

Point::Point(float x, float y)
:	ActionObjectRelay("flash.geom.Point")
,	m_x(x)
,	m_y(y)
{
}

float Point::distance(const Point& pt1, const Point& pt2)
{
	float dx = pt2.m_x - pt1.m_x;
	float dy = pt2.m_y - pt1.m_y;
	return float(std::sqrt(dx * dx + dy * dy));
}

Ref< Point > Point::interpolate(const Point& pt1, const Point& pt2, float f)
{
	float F = float(1) - f;
	return new Point(
		pt1.m_x * F + pt2.m_x * f,
		pt1.m_y * F + pt2.m_y * f
	);
}

Ref< Point > Point::polar(float length, float angle)
{
	return new Point(
		std::sin(angle) * length,
		std::cos(angle) * length
	);
}

Ref< Point > Point::add_1(const Point* pt)
{
	return new Point(
		m_x + pt->m_x,
		m_y + pt->m_y
	);
}

Ref< Point > Point::add_2(float x, float y)
{
	return new Point(
		m_x + x,
		m_y + y
	);
}

Ref< Point > Point::clone(const Point* pt)
{
	return new Point(
		pt->m_x,
		pt->m_y
	);
}

bool Point::equals(const Point* pt)
{
	return m_x == pt->m_x && m_y == pt->m_y;
}

void Point::normalize()
{
	float ln = float(std::sqrt(m_x * m_x + m_y * m_y));
	m_x /= ln;
	m_y /= ln;
}

void Point::offset(float x, float y)
{
	m_x += x;
	m_y += y;
}

void Point::subtract(float x, float y)
{
	m_x -= x;
	m_y -= y;
}

std::wstring Point::toString()
{
	StringOutputStream ss;
	ss << L"(x=" << m_x << L", y=" << m_y << L")";
	return ss.str();
}

float Point::getLength()
{
	return float(std::sqrt(m_x * m_x + m_y * m_y));
}

void Point::setLength(float length)
{
	float ln = float(std::sqrt(m_x * m_x + m_y * m_y));
	float f = length / ln;
	m_x *= f;
	m_y *= f;
}

bool Point::setMember(ActionContext* context, uint32_t memberName, const ActionValue& memberValue)
{
	switch (memberName)
	{
	case ActionContext::IdX:
		m_x = memberValue.getFloat();
		return true;

	case ActionContext::IdY:
		m_y = memberValue.getFloat();
		return true;

	default:
		return false;
	}
}

bool Point::getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue)
{
	switch (memberName)
	{
	case ActionContext::IdX:
		outMemberValue = ActionValue(m_x);
		return true;

	case ActionContext::IdY:
		outMemberValue = ActionValue(m_y);
		return true;

	default:
		return false;
	}
}

	}
}
