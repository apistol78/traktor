#ifndef traktor_ui_EditGtk_H
#define traktor_ui_EditGtk_H

#include "Ui/Gtk/WidgetGtkImpl.h"
#include "Ui/Itf/IEdit.h"

namespace traktor
{
	namespace ui
	{

class EditGtk : public WidgetGtkImpl< IEdit >
{
public:
	EditGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int style);

	virtual void setSelection(int from, int to);

	virtual void getSelection(int& outFrom, int& outTo) const;

	virtual void selectAll();

	virtual void setBorderColor(const Color4ub& borderColor);
};

	}
}

#endif	// traktor_ui_EditGtk_H

