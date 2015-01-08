#ifndef traktor_ui_FormGtk_H
#define traktor_ui_FormGtk_H

#include "Ui/Gtk/WidgetGtkImpl.h"
#include "Ui/Itf/IForm.h"

namespace traktor
{
	namespace ui
	{

class FormGtk : public WidgetGtkImpl< IForm >
{
public:
	FormGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style);

	virtual void setIcon(ui::IBitmap* icon);

	virtual void maximize();

	virtual void minimize();

	virtual void restore();

	virtual bool isMaximized() const;

	virtual bool isMinimized() const;

	virtual void setRect(const Rect& rect);

private:
	void on_remove(Gtk::Widget* widget);
};

	}
}

#endif	// traktor_ui_FormGtk_H

