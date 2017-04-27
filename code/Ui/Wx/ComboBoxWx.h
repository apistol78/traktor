/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ComboBoxWx_H
#define traktor_ui_ComboBoxWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IComboBox.h"

namespace traktor
{
	namespace ui
	{

class ComboBoxWx : public WidgetWxImpl< IComboBox, wxComboBox >
{
public:
	ComboBoxWx(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int style);

	virtual int add(const std::wstring& item);

	virtual bool remove(int index);

	virtual void removeAll();

	virtual int count() const;

	virtual std::wstring get(int index) const;
	
	virtual void select(int index);

	virtual int getSelected() const;

private:
	void onSelected(wxCommandEvent& event);
};

	}
}

#endif	// traktor_ui_ComboBoxWx_H
