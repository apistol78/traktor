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
		int32_t index,
		const ui::Command& command
	);

	void setEnable(bool enable);

	virtual void mouseDown(const ui::Point& position);

	virtual void mouseUp(const ui::Point& position);

	virtual void paint(ui::Canvas& canvas, const ui::Rect& rect);

private:
	Ref< ui::Bitmap > m_bitmap;
	int32_t m_index;
	ui::Command m_command;
	bool m_enable;
	bool m_down;
};

	}
}

#endif	// traktor_amalgam_ButtonCell_H
