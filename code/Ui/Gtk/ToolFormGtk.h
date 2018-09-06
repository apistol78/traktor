#ifndef traktor_ui_ToolFormGtk_H
#define traktor_ui_ToolFormGtk_H

#include "Ui/Gtk/WidgetGtkImpl.h"
#include "Ui/Itf/IToolForm.h"

namespace traktor
{
	namespace ui
	{

class ToolFormGtk : public WidgetGtkImpl< IToolForm >
{
public:
	ToolFormGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void center() T_OVERRIDE T_FINAL;

	virtual void setVisible(bool visible) T_OVERRIDE T_FINAL;

	virtual void setRect(const Rect& rect) T_OVERRIDE T_FINAL;

private:
	GtkWidget* m_window;	
};

	}
}

#endif	// traktor_ui_ToolFormGtk_H

