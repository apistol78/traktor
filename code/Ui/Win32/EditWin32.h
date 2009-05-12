#ifndef traktor_ui_EditWin32_H
#define traktor_ui_EditWin32_H

#include "Ui/Win32/WidgetWin32Impl.h"
#include "Ui/Itf/IEdit.h"

namespace traktor
{
	namespace ui
	{

class EditWin32 : public WidgetWin32Impl< IEdit >
{
public:
	EditWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int style);

	virtual void setSelection(int from, int to);

	virtual void getSelection(int& outFrom, int& outTo) const;

	virtual void selectAll();

	virtual Size getPreferedSize() const;
};

	}
}

#endif	// traktor_ui_EditWin32_H
