#ifndef traktor_ui_DropDownGtk_H
#define traktor_ui_DropDownGtk_H

#include "Ui/Gtk/WidgetGtkImpl.h"
#include "Ui/Itf/IDropDown.h"

namespace traktor
{
	namespace ui
	{

class DropDownGtk : public WidgetGtkImpl< IDropDown >
{
public:
	DropDownGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int style);

	virtual int add(const std::wstring& item);

	virtual bool remove(int index);

	virtual void removeAll();

	virtual int count() const;

	virtual void set(int index, const std::wstring& item);

	virtual std::wstring get(int index) const;
	
	virtual void select(int index);

	virtual int getSelected() const;
};

	}
}

#endif	// traktor_ui_DropDownGtk_H

