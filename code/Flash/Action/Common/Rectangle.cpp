#include "Core/Io/StringOutputStream.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/Common/Point.h"
#include "Flash/Action/Common/Rectangle.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Rectangle", Rectangle, ActionObjectRelay)

Rectangle::Rectangle()
:	ActionObjectRelay("flash.geom.Rectangle")
,	m_x(avm_number_t(0))
,	m_y(avm_number_t(0))
,	m_width(avm_number_t(0))
,	m_height(avm_number_t(0))
{
}

Rectangle::Rectangle(const avm_number_t v[4])
:	ActionObjectRelay("flash.geom.Rectangle")
,	m_x(v[0])
,	m_y(v[1])
,	m_width(v[2])
,	m_height(v[3])
{
}

Ref< Rectangle > Rectangle::clone(const Rectangle* s)
{
	Ref< Rectangle > c = new Rectangle();
	c->m_x = s->m_x;
	c->m_y = s->m_y;
	c->m_width = s->m_width;
	c->m_height = s->m_height;
	return c;
}

bool Rectangle::contains(avm_number_t x, avm_number_t y)
{
	return x >= m_x && y >= m_y && x <= m_x + m_width && y <= m_y + m_height;
}

bool Rectangle::containsPoint(const Point* pt)
{
	return contains(pt->m_x, pt->m_y);
}

bool Rectangle::containsRectangle(const Rectangle* r)
{
	T_FATAL_ERROR;
	return false;
}

bool Rectangle::equals(const Rectangle* s)
{
	return m_x == s->m_x && m_y == s->m_y && m_width == s->m_width && m_height == s->m_height;
}

void Rectangle::inflate(avm_number_t x, avm_number_t y)
{
	m_x -= x;
	m_y -= y;
	m_width += x * avm_number_t(2);
	m_height += y * avm_number_t(2);
}

void Rectangle::inflatePoint(const Point* pt)
{
	inflate(pt->m_x, pt->m_y);
}

void Rectangle::intersection()
{
	T_FATAL_ERROR;
}

void Rectangle::intersects()
{
	T_FATAL_ERROR;
}

bool Rectangle::isEmpty()
{
	return m_width <= avm_number_t(0) || m_height <= avm_number_t(0);
}

void Rectangle::offset(avm_number_t x, avm_number_t y)
{
	m_x += x;
	m_y += y;
}

void Rectangle::offsetPoint(const Point* pt)
{
	offset(pt->m_x, pt->m_y);
}

void Rectangle::setEmpty()
{
	m_x =
	m_y =
	m_width =
	m_height = avm_number_t(0);
}

std::wstring Rectangle::toString()
{
	StringOutputStream ss;
	ss << L"(x=" << m_x << L", y=" << m_y << L", w=" << m_width << L", h=" << m_height << L")";
	return ss.str();
}

void Rectangle::union_()
{
	T_FATAL_ERROR;
}

avm_number_t Rectangle::getBottom()
{
	return m_y + m_height;
}

void Rectangle::setBottom(avm_number_t v)
{
	m_height = v - m_y;
}

Ref< Point > Rectangle::getBottomRight()
{
	return new Point(m_x + m_width, m_y + m_height);
}

void Rectangle::setBottomRight(const Point* pt)
{
	setRight(pt->m_x);
	setBottom(pt->m_y);
}

avm_number_t Rectangle::getLeft()
{
	return m_x;
}

void Rectangle::setLeft(avm_number_t v)
{
	m_width = getRight() - v;
	m_x = v;
}

avm_number_t Rectangle::getRight()
{
	return m_x + m_width;
}

void Rectangle::setRight(avm_number_t v)
{
	m_width = v - m_x;
}

Ref< Point > Rectangle::getSize()
{
	return new Point(m_width, m_height);
}

void Rectangle::setSize(const Point* pt)
{
	m_width = pt->m_x;
	m_height = pt->m_y;
}

avm_number_t Rectangle::getTop()
{
	return m_y;
}

void Rectangle::setTop(avm_number_t v)
{
	m_height = getBottom() - v;
	m_y = v;
}

Ref< Point > Rectangle::getTopLeft()
{
	return new Point(m_x, m_y);
}

void Rectangle::setTopLeft(const Point* pt)
{
	setLeft(pt->m_x);
	setTop(pt->m_y);
}

bool Rectangle::setMember(ActionContext* context, uint32_t memberName, const ActionValue& memberValue)
{
	switch (memberName)
	{
	case ActionContext::IdX:
		m_x = memberValue.getNumber();
		return true;

	case ActionContext::IdY:
		m_y = memberValue.getNumber();
		return true;

	case ActionContext::IdWidth:
		m_width = memberValue.getNumber();
		return true;

	case ActionContext::IdHeight:
		m_height = memberValue.getNumber();
		return true;

	default:
		return false;
	}
}

bool Rectangle::getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue)
{
	switch (memberName)
	{
	case ActionContext::IdX:
		outMemberValue = ActionValue(m_x);
		return true;

	case ActionContext::IdY:
		outMemberValue = ActionValue(m_y);
		return true;

	case ActionContext::IdWidth:
		outMemberValue = ActionValue(m_width);
		return true;

	case ActionContext::IdHeight:
		outMemberValue = ActionValue(m_height);
		return true;

	default:
		return false;
	}
}

	}
}
