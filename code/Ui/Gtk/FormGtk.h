#ifndef traktor_ui_FormGtk_H
#define traktor_ui_FormGtk_H

#include "Ui/Gtk/WindowGtkImpl.h"
#include "Ui/Itf/IForm.h"

namespace traktor
{
	namespace ui
	{

class FormGtk : public WindowGtkImpl< IForm >
{
public:
	FormGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::string& text, int width, int height, int style);

	virtual void setIcon(drawing::Image* icon);

	virtual IMenuBar* createMenuBar(EventSubject* owner);

	virtual IToolBar* createToolBar(EventSubject* owner);

	virtual IStatusBar* createStatusBar(EventSubject* owner);

	virtual void maximize();

	virtual void minimize();

	virtual void restore();

	virtual bool isMaximized() const;

	virtual bool isMinimized() const;

	virtual Rect getInnerRect() const;

private:
	void on_remove(Gtk::Widget* widget);

	void on_size_allocate(Gtk::Allocation& allocation);
};

	}
}

#endif	// traktor_ui_FormGtk_H

