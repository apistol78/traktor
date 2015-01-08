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
	Internal* parentInternal = static_cast< Internal* >(parent->getInternalHandle());
	T_FATAL_ASSERT(parentInternal);

	Gtk::Fixed* container = new Gtk::Fixed();
	parentInternal->container->put(*container, 0, 0);

	Gtk::ComboBox* comboBox = new Gtk::ComboBox();

	container->put(*comboBox, 0, 0);
	comboBox->show();

	m_internal.container = container;
	m_internal.widget = comboBox;

	return WidgetGtkImpl< IDropDown >::create();
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

