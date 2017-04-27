/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ListBoxWx_H
#define traktor_ui_ListBoxWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IListBox.h"

namespace traktor
{
	namespace ui
	{

class ListBoxWx : public WidgetWxImpl< IListBox, wxListBox >
{
public:
	ListBoxWx(EventSubject* owner);

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

private:
	void onSelected(wxCommandEvent& event);
};

	}
}

#endif	// traktor_ui_ListBoxWx_H
