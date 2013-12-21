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
	// @fixme Not safe, we must check so the internal handle in fact are a Gtk::Fixed container. Also need to ensure cleanup of m_parentContainer.
	m_parentContainer = static_cast< Gtk::Fixed* >(parent->getInternalHandle());
	if (!m_parentContainer)
		return false;

	Gtk::ComboBox* comboBox = new Gtk::ComboBox();

	m_parentContainer->put(*comboBox, 0, 0);

	comboBox->show();

	m_widget = comboBox;

	return true;
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

