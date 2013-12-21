#include "Core/Misc/TString.h"
#include "Ui/Gtk/EditGtk.h"
#include "Ui/Events/CommandEvent.h"

namespace traktor
{
	namespace ui
	{

EditGtk::EditGtk(EventSubject* owner)
:	WidgetGtkImpl< IEdit >(owner)
{
}

bool EditGtk::create(IWidget* parent, const std::wstring& text, int style)
{
	// @fixme Not safe, we must check so the internal handle in fact are a Gtk::Fixed container. Also need to ensure cleanup of m_parentContainer.
	m_parentContainer = static_cast< Gtk::Fixed* >(parent->getInternalHandle());
	if (!m_parentContainer)
		return false;

	Gtk::Entry* entry = new Gtk::Entry();
	//button->signal_clicked().connect(sigc::mem_fun(*this, &EditGtk::on_button_clicked));

	m_parentContainer->put(*entry, 0, 0);

	entry->show();

	m_widget = entry;

	return true;
}

void EditGtk::setSelection(int from, int to)
{
}

void EditGtk::getSelection(int& outFrom, int& outTo) const
{
}

void EditGtk::selectAll()
{
}

	}
}

