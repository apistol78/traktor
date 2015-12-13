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
	T_RTTI_CLASS;

public:
	ButtonCell(
		ui::Bitmap* bitmap,
		const ui::Command& command
	);

	void setEnable(bool enable);

	virtual void mouseDown(ui::MouseButtonDownEvent* event, const ui::Point& position) T_FINAL;

	virtual void mouseUp(ui::MouseButtonUpEvent* event, const ui::Point& position) T_FINAL;

	virtual void paint(ui::Canvas& canvas, const ui::Rect& rect) T_FINAL;

private:
	Ref< ui::Bitmap > m_bitmap;
	ui::Command m_command;
	bool m_enable;
	bool m_down;
};

	}
}

#endif	// traktor_amalgam_ButtonCell_H
