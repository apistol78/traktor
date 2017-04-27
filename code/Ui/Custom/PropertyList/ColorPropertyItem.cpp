/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <sstream>
#include "Ui/Application.h"
#include "Ui/Command.h"
#include "Ui/Custom/PropertyList/ColorPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ColorPropertyItem", ColorPropertyItem, PropertyItem)

ColorPropertyItem::ColorPropertyItem(const std::wstring& text, const Color4ub& value)
:	PropertyItem(text)
,	m_value(value)
,	m_rcColor(0, 0, 0, 0)
{
}

void ColorPropertyItem::setValue(const Color4ub& value)
{
	m_value = value;
}

const Color4ub& ColorPropertyItem::getValue() const
{
	return m_value;
}

void ColorPropertyItem::mouseButtonUp(MouseButtonUpEvent* event)
{
	if (m_rcColor.inside(event->getPosition()))
		notifyCommand(Command(L"Property.Edit"));
}

void ColorPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	// Format color as string.
	std::wstringstream ss;
	ss << m_value.r << L", " << m_value.g << L", " << m_value.b << L", " << m_value.a;
	canvas.drawText(rc.inflate(-2, 0).offset(scaleBySystemDPI(22), 0), ss.str(), AnLeft, AnCenter);

	// Ignore alpha when drawing color preview.
	Color4ub previewColor = m_value;
	previewColor.a = 255;

	canvas.setBackground(previewColor);
	canvas.setForeground(Color4ub(0, 0, 0));

	// Draw color preview with a black border.
	m_rcColor = Rect(rc.left + 2, rc.top + 2, rc.left + scaleBySystemDPI(22), rc.bottom - 2);
	canvas.fillRect(m_rcColor);
	canvas.drawRect(m_rcColor);
}

		}
	}
}
