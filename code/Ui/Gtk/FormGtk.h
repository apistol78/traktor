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

	virtual void setIcon(ISystemBitmap* icon);

	virtual void maximize();

	virtual void minimize();

	virtual void restore();

	virtual bool isMaximized() const;

	virtual bool isMinimized() const;

	virtual void hideProgress();

	virtual void showProgress(int32_t current, int32_t total);

	virtual void setVisible(bool visible) T_OVERRIDE T_FINAL;

private:
	GtkWidget* m_window;

	static gboolean signal_remove(GtkWidget* widget, GdkEvent* event, gpointer data);
};

	}
}

#endif	// traktor_ui_FormGtk_H

