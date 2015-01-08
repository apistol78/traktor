#include "Ui/ScrollBar.h"
#include "Ui/Gtk/ScrollBarGtk.h"

namespace traktor
{
	namespace ui
	{

ScrollBarGtk::ScrollBarGtk(EventSubject* owner)
:	WidgetGtkImpl< IScrollBar >(owner)
{
}

bool ScrollBarGtk::create(IWidget* parent, int style)
{
	Internal* parentInternal = static_cast< Internal* >(parent->getInternalHandle());
	T_FATAL_ASSERT(parentInternal);

	m_adjustment = Gtk::Adjustment::create(0.0, 0.0, 0.0);

	Gtk::Fixed* container = new Gtk::Fixed();
	parentInternal->container->put(*container, 0, 0);

	Gtk::Scrollbar* scrollbar = new Gtk::Scrollbar(m_adjustment, (style & ScrollBar::WsVertical) != 0 ? Gtk::ORIENTATION_VERTICAL : Gtk::ORIENTATION_HORIZONTAL);
	container->put(*scrollbar, 0, 0);
	scrollbar->show();

	m_internal.container = container;
	m_internal.widget = scrollbar;

	return WidgetGtkImpl< IScrollBar >::create();
}

void ScrollBarGtk::setRange(int range)
{
}

int ScrollBarGtk::getRange() const
{
	return 0;
}

void ScrollBarGtk::setPage(int page)
{
}

int ScrollBarGtk::getPage() const
{
	return 0;
}

void ScrollBarGtk::setPosition(int position)
{
}

int ScrollBarGtk::getPosition() const
{
	return 0;
}

	}
}


