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
	// @fixme Not safe, we must check so the internal handle in fact are a Gtk::Fixed container. Also need to ensure cleanup of m_parentContainer.
	m_parentContainer = static_cast< Gtk::Fixed* >(parent->getInternalHandle());
	if (!m_parentContainer)
		return false;

	m_adjustment = Gtk::Adjustment::create(0.0, 0.0, 0.0);

	Gtk::Scrollbar* scrollbar = new Gtk::Scrollbar(m_adjustment, (style & ScrollBar::WsVertical) != 0 ? Gtk::ORIENTATION_VERTICAL : Gtk::ORIENTATION_HORIZONTAL);

	m_parentContainer->put(*scrollbar, 0, 0);

	scrollbar->show();

	m_widget = scrollbar;

	return true;
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


