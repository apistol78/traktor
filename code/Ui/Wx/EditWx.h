#ifndef traktor_ui_EditWx_H
#define traktor_ui_EditWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IEdit.h"

namespace traktor
{
	namespace ui
	{

class EditWx : public WidgetWxImpl< IEdit, wxTextCtrl >
{
public:
	EditWx(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int style);

	virtual void setSelection(int from, int to);

	virtual void getSelection(int& outFrom, int& outTo) const;

	virtual void selectAll();

	virtual void setText(const std::wstring& text);

	virtual std::wstring getText() const;
};

	}
}

#endif	// traktor_ui_EditWx_H
