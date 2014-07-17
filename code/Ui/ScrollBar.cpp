#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/ScrollBar.h"
#include "Ui/Itf/IScrollBar.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ScrollBar", ScrollBar, ui::Widget)

bool ScrollBar::create(Widget* parent, int style)
{
	if (!parent)
		return false;

	IScrollBar* scrollBar = Application::getInstance()->getWidgetFactory()->createScrollBar(this);
	if (!scrollBar)
	{
		log::error << L"Failed to create native widget peer (ScrollBar)" << Endl;
		return false;
	}

	if (!scrollBar->create(parent->getIWidget(), style))
	{
		scrollBar->destroy();
		return false;
	}

	m_widget = scrollBar;

	return Widget::create(parent, style);
}

void ScrollBar::setRange(int range)
{
	T_ASSERT (m_widget);
	static_cast< IScrollBar* >(m_widget)->setRange(range);
}

int ScrollBar::getRange() const
{
	T_ASSERT (m_widget);
	return static_cast< IScrollBar* >(m_widget)->getRange();
}

void ScrollBar::setPage(int page)
{
	T_ASSERT (m_widget);
	static_cast< IScrollBar* >(m_widget)->setPage(page);
}

int ScrollBar::getPage() const
{
	T_ASSERT (m_widget);
	return static_cast< IScrollBar* >(m_widget)->getPage();
}

void ScrollBar::setPosition(int position)
{
	T_ASSERT (m_widget);
	static_cast< IScrollBar* >(m_widget)->setPosition(position);
}

int ScrollBar::getPosition() const
{
	T_ASSERT (m_widget);
	return static_cast< IScrollBar* >(m_widget)->getPosition();
}

	}
}
