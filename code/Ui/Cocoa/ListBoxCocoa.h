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

	virtual bool create(IWidget* parent, int style);

	virtual int add(const std::wstring& item);

	virtual bool remove(int index);

	virtual void removeAll();

	virtual int count() const;

	virtual void set(int index, const std::wstring& item);

	virtual std::wstring get(int index) const;
	
	virtual void select(int index);

	virtual bool selected(int index) const;

	virtual Rect getItemRect(int index) const;
	
	// IListDataCallback
	
	virtual int listCount() const;
	
	virtual std::wstring listValue(NSTableColumn* tableColumn, int index) const;
	
	// ITargetProxyCallback
	
	virtual void targetProxy_Action(void* controlId);
	
	virtual void targetProxy_doubleAction(void* controlId);
		
private:
	std::vector< std::wstring > m_items;
};

	}
}

#endif	// traktor_ui_ListBoxCocoa_H
