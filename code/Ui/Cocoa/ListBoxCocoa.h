#ifndef traktor_ui_ListBoxCocoa_H
#define traktor_ui_ListBoxCocoa_H

#import "Ui/Cocoa/NSListDataSource.h"

#include <vector>
#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/IListBox.h"

namespace traktor
{
	namespace ui
	{

class ListBoxCocoa
:	public WidgetCocoaImpl< IListBox, NSTableView, NSScrollView >
,	public IListDataCallback
,	public ITargetProxyCallback
{
public:
	ListBoxCocoa(EventSubject* owner);
	
	// IListBox

	virtual bool create(IWidget* parent, int style) T_OVERRIDE T_FINAL;

	virtual int add(const std::wstring& item) T_OVERRIDE T_FINAL;

	virtual bool remove(int index) T_OVERRIDE T_FINAL;

	virtual void removeAll() T_OVERRIDE T_FINAL;

	virtual int count() const T_OVERRIDE T_FINAL;

	virtual void set(int index, const std::wstring& item) T_OVERRIDE T_FINAL;

	virtual std::wstring get(int index) const T_OVERRIDE T_FINAL;
	
	virtual void select(int index) T_OVERRIDE T_FINAL;

	virtual bool selected(int index) const T_OVERRIDE T_FINAL;

	virtual Rect getItemRect(int index) const T_OVERRIDE T_FINAL;
	
	// IListDataCallback
	
	virtual int listCount() const T_OVERRIDE T_FINAL;
	
	virtual std::wstring listValue(NSTableColumn* tableColumn, int index) const T_OVERRIDE T_FINAL;
	
	// ITargetProxyCallback
	
	virtual void targetProxy_Action(void* controlId) T_OVERRIDE T_FINAL;
	
	virtual void targetProxy_doubleAction(void* controlId) T_OVERRIDE T_FINAL;
		
private:
	std::vector< std::wstring > m_items;
};

	}
}

#endif	// traktor_ui_ListBoxCocoa_H
