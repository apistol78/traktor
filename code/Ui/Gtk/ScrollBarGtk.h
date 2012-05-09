#ifndef traktor_ui_ScrollBarGtk_H
#define traktor_ui_ScrollBarGtk_H

#include "Ui/Gtk/WidgetGtkImpl.h"
#include "Ui/Itf/IScrollBar.h"

namespace traktor
{
	namespace ui
	{

class ScrollBarGtk : public WidgetGtkImpl< IScrollBar >
{
public:
	ScrollBarGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

	virtual void setRange(int range);

	virtual int getRange() const;

	virtual void setPage(int page);

	virtual int getPage() const;

	virtual void setPosition(int position);

	virtual int getPosition() const;

private:
	Glib::RefPtr< Gtk::Adjustment > m_adjustment;
};

	}
}

#endif	// traktor_ui_ScrollBarGtk_H
