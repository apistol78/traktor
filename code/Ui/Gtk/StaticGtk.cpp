#include "Core/Misc/TString.h"
#include "Ui/Gtk/StaticGtk.h"

namespace traktor
{
	namespace ui
	{

StaticGtk::StaticGtk(EventSubject* owner)
:	WidgetGtkImpl< IStatic >(owner)
{
}

bool StaticGtk::create(IWidget* parent, const std::wstring& text)
{
	// @fixme Not safe, we must check so the internal handle in fact are a Gtk::Fixed container. Also need to ensure cleanup of m_parentContainer.
	m_parentContainer = static_cast< Gtk::Fixed* >(parent->getInternalHandle());
	if (!m_parentContainer)
		return false;

	Gtk::Label* label = new Gtk::Label(wstombs(text).c_str());

	m_parentContainer->put(*label, 0, 0);

	label->show();

	m_widget = label;

	return true;
}

	}
}

