/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Html/Element.h"
#include "Webber/Widget.h"

namespace traktor::wbr
{
	namespace
	{

int32_t allocateWidgetId()
{
	static int32_t s_widgetNextId = 1;
	return s_widgetNextId++;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Widget", Widget, Object)

Widget::Widget()
:	m_id(allocateWidgetId())
{
}

void Widget::setStyleClass(const std::wstring& styleClass)
{
	m_styleClass = styleClass;
}

void Widget::consumeValues(const std::map< int32_t, std::wstring >& values)
{
}

void Widget::consume(int32_t senderId, int32_t action)
{
}

html::Element* Widget::createElement(const std::wstring& name, const std::wstring& className, html::Element* parent) const
{
	auto element = new html::Element(name);
	element->setAttribute(L"id", L"_" + toString(m_id));
	element->setAttribute(L"class", m_styleClass.empty() ? className : className + L" " + m_styleClass);
	parent->addChild(element);
	return element;
}

}
