#ifndef traktor_ui_CheckBoxGtk_H
#define traktor_ui_CheckBoxGtk_H

#include "Ui/Gtk/WidgetGtkImpl.h"
#include "Ui/Itf/ICheckBox.h"

namespace traktor
{
	namespace ui
	{

class CheckBoxGtk : public WidgetGtkImpl< ICheckBox >
{
public:
	CheckBoxGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, bool checked);

	virtual void setChecked(bool checked);

	virtual bool isChecked() const;

private:
	void onClicked();
};

	}
}

#endif	// traktor_ui_CheckBoxGtk_H

