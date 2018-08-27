#ifndef traktor_ui_DialogGtk_H
#define traktor_ui_DialogGtk_H

#include "Ui/Gtk/WidgetGtkImpl.h"
#include "Ui/Itf/IDialog.h"

namespace traktor
{
	namespace ui
	{

class DialogGtk : public WidgetGtkImpl< IDialog >
{
public:
	DialogGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setIcon(ISystemBitmap* icon);

	virtual int showModal();

	virtual void endModal(int result);

	virtual void setMinSize(const Size& minSize);

	virtual void setVisible(bool visible) T_OVERRIDE T_FINAL;

private:
	GtkWidget* m_window;
	int32_t m_result;
	bool m_modal;

	static gboolean signal_remove(GtkWidget* widget, GdkEvent* event, gpointer data);
};

	}
}

#endif	// traktor_ui_DialogGtk_H
