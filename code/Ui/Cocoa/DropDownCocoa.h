#ifndef traktor_ui_DropDownCocoa_H
#define traktor_ui_DropDownCocoa_H

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/IDropDown.h"

namespace traktor
{
	namespace ui
	{

class DropDownCocoa : public WidgetCocoaImpl< IDropDown, NSComboBox >
{
public:
	DropDownCocoa(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int style);

	virtual int add(const std::wstring& item);

	virtual bool remove(int index);

	virtual void removeAll();

	virtual int count() const;

	virtual std::wstring get(int index) const;
	
	virtual void select(int index);

	virtual int getSelected() const;
};

	}
}

#endif	// traktor_ui_DropDownCocoa_H
