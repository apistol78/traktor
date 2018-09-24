/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/PropertyList/PropertyList.h"
#include "Ui/PropertyList/StaticPropertyItem.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.StaticPropertyItem", StaticPropertyItem, PropertyItem)

StaticPropertyItem::StaticPropertyItem(const std::wstring& text, const std::wstring& value) :
	PropertyItem(text),
	m_value(value)
{
}

void StaticPropertyItem::setValue(const std::wstring& value)
{
	m_value = value;
}

const std::wstring& StaticPropertyItem::getValue() const
{
	return m_value;
}

void StaticPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	canvas.drawText(rc.inflate(-2, 0), m_value, AnLeft, AnCenter);
}

	}
}
