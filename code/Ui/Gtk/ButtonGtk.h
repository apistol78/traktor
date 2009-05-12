#ifndef traktor_ui_ButtonGtk_H
#define traktor_ui_ButtonGtk_H

#include "Ui/Gtk/WidgetGtkImpl.h"
#include "Ui/Itf/IButton.h"

namespace traktor
{
	namespace ui
	{

class ButtonGtk : public WidgetGtkImpl< IButton >
{
public:
	ButtonGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::string& text, int id, int style);

	virtual void setState(bool state);

	virtual bool getState() const;

private:
	int m_id;

	void on_button_clicked();
};

	}
}

#endif	// traktor_ui_ButtonGtk_H

