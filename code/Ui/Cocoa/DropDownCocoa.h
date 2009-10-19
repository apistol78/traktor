#ifndef traktor_ui_DropDownCocoa_H
#define traktor_ui_DropDownCocoa_H

#import "Ui/Cocoa/NSComboBoxDataSource.h"

#include <vector>
#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/IDropDown.h"

namespace traktor
{
	namespace ui
	{

class DropDownCocoa
:	public WidgetCocoaImpl< IDropDown, NSComboBox >
,	public IComboBoxDataCallback
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
	
	// IComboBoxDataCallback
	
	virtual int comboBox_indexOf(const std::wstring& value) const;
	
	virtual std::wstring comboBox_valueAt(int index) const;
	
	virtual int comboBox_numberOfItems() const;
	
private:
	std::vector< std::wstring > m_items;
};

	}
}

#endif	// traktor_ui_DropDownCocoa_H
