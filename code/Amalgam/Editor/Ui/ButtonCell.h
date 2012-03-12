#ifndef traktor_amalgam_ButtonCell_H
#define traktor_amalgam_ButtonCell_H

#include "Ui/Bitmap.h"
#include "Ui/Command.h"
#include "Ui/Custom/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace amalgam
	{

class ButtonCell : public ui::custom::AutoWidgetCell
{
public:
	ButtonCell(
		ui::Bitmap* bitmap,
		int32_t index,
		bool enable,
		int32_t eventId,
		Object* eventItem,
		const ui::Command& eventCommand
	);

	void setEnable(bool enable);

	virtual void mouseDown(ui::custom::AutoWidget* widget, const ui::Point& position);

	virtual void mouseUp(ui::custom::AutoWidget* widget, const ui::Point& position);

	virtual void paint(ui::custom::AutoWidget* widget, ui::Canvas& canvas, const ui::Rect& rect);

private:
	Ref< ui::Bitmap > m_bitmap;
	int32_t m_index;
	bool m_enable;
	bool m_down;
	int32_t m_eventId;
	Ref< Object > m_eventItem;
	ui::Command m_eventCommand;
};

	}
}

#endif	// traktor_amalgam_ButtonCell_H
