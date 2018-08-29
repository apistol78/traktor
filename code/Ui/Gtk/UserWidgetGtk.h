#ifndef traktor_ui_UserWidgetGtk_H
#define traktor_ui_UserWidgetGtk_H

#include "Ui/Gtk/WidgetGtkImpl.h"
#include "Ui/Itf/IUserWidget.h"

namespace traktor
{
	namespace ui
	{

class UserWidgetGtk : public WidgetGtkImpl< IUserWidget >
{
public:
	UserWidgetGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

	virtual Size getTextExtent(const std::wstring& text) const T_OVERRIDE T_FINAL;

	virtual void setFont(const Font& font) T_OVERRIDE T_FINAL;

	virtual Font getFont() const T_OVERRIDE T_FINAL;

private:
	BareContainer* m_parent;
	Font m_font;

	static gboolean signal_draw(GtkWidget* widget, cairo_t* cr, gpointer data);
};

	}
}

#endif	// traktor_ui_UserWidgetGtk_H

