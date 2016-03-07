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
,	m_x(avm_number_t(0))
,	m_y(avm_number_t(0))
{
}

Point::Point(avm_number_t x, avm_number_t y)
:	ActionObjectRelay("flash.geom.Point")
,	m_x(x)
,	m_y(y)
{
}

avm_number_t Point::distance(const Point& pt1, const Point& pt2)
{
	avm_number_t dx = pt2.m_x - pt1.m_x;
	avm_number_t dy = pt2.m_y - pt1.m_y;
	return avm_number_t(sqrtf(dx * dx + dy * dy));
}

Ref< Point > Point::interpolate(const Point& pt1, const Point& pt2, avm_number_t f)
{
	avm_number_t F = avm_number_t(1) - f;
	return new Point(
		pt1.m_x * F + pt2.m_x * f,
		pt1.m_y * F + pt2.m_y * f
	);
}

Ref< Point > Point::polar(avm_number_t length, avm_number_t angle)
{
	return new Point(
		sin(angle) * length,
		cos(angle) * length
	);
}

Ref< Point > Point::add_1(const Point* pt)
{
	return new Point(
		m_x + pt->m_x,
		m_y + pt->m_y
	);
}

Ref< Point > Point::add_2(avm_number_t x, avm_number_t y)
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
	avm_number_t ln = avm_number_t(sqrtf(m_x * m_x + m_y * m_y));
	m_x /= ln;
	m_y /= ln;
}

void Point::offset(avm_number_t x, avm_number_t y)
{
	m_x += x;
	m_y += y;
}

void Point::subtract(avm_number_t x, avm_number_t y)
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

avm_number_t Point::getLength()
{
	return avm_number_t(sqrtf(m_x * m_x + m_y * m_y));
}

void Point::setLength(avm_number_t length)
{
	avm_number_t ln = avm_number_t(sqrtf(m_x * m_x + m_y * m_y));
	avm_number_t f = length / ln;
	m_x *= f;
	m_y *= f;
}

bool Point::setMember(ActionContext* context, uint32_t memberName, const ActionValue& memberValue)
{
	switch (memberName)
	{
	case ActionContext::IdX:
		m_x = memberValue.getNumber();
		return true;

	case ActionContext::IdY:
		m_y = memberValue.getNumber();
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
