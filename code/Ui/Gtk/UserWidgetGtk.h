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

	virtual void setFont(const Font& font) T_OVERRIDE T_FINAL;

	virtual Font getFont() const T_OVERRIDE T_FINAL;

	// IFontMetric

	virtual void getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const T_OVERRIDE T_FINAL;

	virtual int32_t getAdvance(wchar_t ch, wchar_t next) const T_OVERRIDE T_FINAL;

	virtual int32_t getLineSpacing() const T_OVERRIDE T_FINAL;

	virtual Size getExtent(const std::wstring& text) const T_OVERRIDE T_FINAL;

private:
	BareContainer* m_parent;
	Font m_font;

	static gboolean signal_draw(GtkWidget* widget, cairo_t* cr, gpointer data);
};

	}
}

#endif	// traktor_ui_UserWidgetGtk_H

