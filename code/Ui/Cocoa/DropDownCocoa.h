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

	virtual bool create(IWidget* parent, const std::wstring& text, int style) T_OVERRIDE T_FINAL;

	virtual int add(const std::wstring& item) T_OVERRIDE T_FINAL;

	virtual bool remove(int index) T_OVERRIDE T_FINAL;

	virtual void removeAll() T_OVERRIDE T_FINAL;

	virtual int count() const T_OVERRIDE T_FINAL;
	
	virtual void set(int index, const std::wstring& item) T_OVERRIDE T_FINAL;

	virtual std::wstring get(int index) const T_OVERRIDE T_FINAL;
	
	virtual void select(int index) T_OVERRIDE T_FINAL;

	virtual int getSelected() const T_OVERRIDE T_FINAL;
	
	// IComboBoxDataCallback
	
	virtual int comboBox_indexOf(const std::wstring& value) const T_OVERRIDE T_FINAL;
	
	virtual std::wstring comboBox_valueAt(int index) const T_OVERRIDE T_FINAL;
	
	virtual int comboBox_numberOfItems() const T_OVERRIDE T_FINAL;
	
private:
	std::vector< std::wstring > m_items;
};

	}
}

#endif	// traktor_ui_DropDownCocoa_H
