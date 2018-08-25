#include "Core/Misc/TString.h"
#include "Ui/Gtk/EditGtk.h"

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
	Warp* p = static_cast< Warp* >(parent->getInternalHandle());
	T_FATAL_ASSERT(p != nullptr);

	m_warp.widget = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(p->widget), m_warp.widget);

	m_text = text;

	return WidgetGtkImpl< IEdit >::create(parent);
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

void EditGtk::setBorderColor(const Color4ub& borderColor)
{
}

	}
}

