#ifndef EditList_H
#define EditList_H

#include <Ui/ListBox.h>
#include <Ui/Edit.h>

class EditList : public traktor::ui::ListBox
{
public:
	bool create(traktor::ui::Widget* parent);

	void addEditEventHandler(traktor::ui::EventHandler* eventHandler);

private:
	traktor::Ref< traktor::ui::Edit > m_editItem;
	int m_editId;

	void eventDoubleClick(traktor::ui::Event* event);

	void eventEditFocus(traktor::ui::Event* event);
};

#endif	// EditList_H
