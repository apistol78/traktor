#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/ListBox/ListBox.h"
#include "Ui/ListBox/ListBoxItem.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ListBoxItem", ListBoxItem, AutoWidgetCell)

ListBoxItem::ListBoxItem()
:	m_bgcolor(0, 0, 0, 0)
,	m_selected(false)
{
}

void ListBoxItem::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& ListBoxItem::getText() const
{
	return m_text;
}

void ListBoxItem::setBackgroundColor(const Color4ub& bgcolor)
{
	m_bgcolor = bgcolor;
}

const Color4ub& ListBoxItem::getBackgroundColor() const
{
	return m_bgcolor;
}

bool ListBoxItem::setSelected(bool selected)
{
	if (selected != m_selected)
	{
		m_selected = selected;
		requestWidgetUpdate();
		return true;
	}
	else
		return false;
}

bool ListBoxItem::isSelected() const
{
	return m_selected;
}

void ListBoxItem::paint(Canvas& canvas, const Rect& rect)
{
	const StyleSheet* ss = getWidget< ListBox >()->getStyleSheet();

	if (m_selected)
	{
		canvas.setBackground(ss->getColor(getWidget< ListBox >(), L"item-background-color-selected"));
		canvas.fillRect(rect);
	}
	else if (m_bgcolor.a != 0)
	{
		canvas.setBackground(m_bgcolor);
		canvas.fillRect(rect);
	}

	canvas.setForeground(ss->getColor(getWidget< ListBox >(), m_selected ? L"item-color-selected" : L"color"));
	canvas.drawText(rect, m_text, AnLeft, AnCenter);
}

	}
}
