#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/ListBox/ListBox.h"
#include "Ui/Custom/ListBox/ListBoxItem.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
		
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ListBoxItem", ListBoxItem, AutoWidgetCell)

ListBoxItem::ListBoxItem()
:	m_selected(false)
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

bool ListBoxItem::setSelected(bool selected)
{
	if (selected != m_selected)
	{
		m_selected = selected;
		requestUpdate();
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
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	if (m_selected)
	{
		canvas.setBackground(ss->getColor(getWidget< ListBox >(), L"item-background-color-selected"));
		canvas.fillRect(rect);
	}

	canvas.setForeground(ss->getColor(getWidget< ListBox >(), m_selected ? L"item-color-selected" : L"color"));
	canvas.drawText(rect, m_text, AnLeft, AnCenter);
}

		}
	}
}
