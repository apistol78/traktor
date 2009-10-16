#ifndef traktor_ui_EditCocoa_H
#define traktor_ui_EditCocoa_H

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/IEdit.h"

namespace traktor
{
	namespace ui
	{

class EditCocoa : public WidgetCocoaImpl< IEdit, NSTextField >
{
public:
	EditCocoa(EventSubject* owner);
	
	// IEdit

	virtual bool create(IWidget* parent, const std::wstring& text, int style);

	virtual void setSelection(int from, int to);

	virtual void getSelection(int& outFrom, int& outTo) const;

	virtual void selectAll();

	virtual void setText(const std::wstring& text);

	virtual std::wstring getText() const;
};

	}
}

#endif	// traktor_ui_EditCocoa_H
