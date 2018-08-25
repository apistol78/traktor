#include "Core/Misc/TString.h"
#include "Ui/Gtk/DropDownGtk.h"

namespace traktor
{
	namespace ui
	{

DropDownGtk::DropDownGtk(EventSubject* owner)
:	WidgetGtkImpl< IDropDown >(owner)
{
}

bool DropDownGtk::create(IWidget* parent, const std::wstring& text, int style)
{
	Warp* p = static_cast< Warp* >(parent->getInternalHandle());
	T_FATAL_ASSERT(p != nullptr);

	m_warp.widget = gtk_combo_box_new();
	gtk_container_add(GTK_CONTAINER(p->widget), m_warp.widget);

	m_text = text;

	return WidgetGtkImpl< IDropDown >::create(parent);
}

int DropDownGtk::add(const std::wstring& item)
{
	return 0;
}

bool DropDownGtk::remove(int index)
{
	return false;
}

void DropDownGtk::removeAll()
{
}

int DropDownGtk::count() const
{
	return 0;
}

void DropDownGtk::set(int index, const std::wstring& item)
{
}

std::wstring DropDownGtk::get(int index) const
{
	return L"";
}

void DropDownGtk::select(int index)
{
}

int DropDownGtk::getSelected() const
{
	return -1;
}

	}
}

