#ifndef traktor_ui_DialogGtk_H
#define traktor_ui_DialogGtk_H

#include "Ui/Gtk/WindowGtkImpl.h"
#include "Ui/Itf/IDialog.h"

namespace traktor
{
	namespace ui
	{

class DialogGtk : public WindowGtkImpl< IDialog >
{
public:
	DialogGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style);

	virtual void setIcon(drawing::Image* icon);

	virtual int showModal();

	virtual void endModal(int result);

	virtual void setMinSize(const Size& minSize);

private:
	void on_remove(Gtk::Widget* widget);

	void on_size_allocate(Gtk::Allocation& allocation);
};

	}
}

#endif	// traktor_ui_DialogGtk_H
