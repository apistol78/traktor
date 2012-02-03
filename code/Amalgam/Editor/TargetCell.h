#ifndef traktor_amalgam_TargetCell_H
#define traktor_amalgam_TargetCell_H

#include "Ui/Custom/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace ui
	{

class Bitmap;

	}

	namespace amalgam
	{

class ButtonCell;
class DropListCell;
class HostEnumerator;
class ProgressCell;
class TargetInstance;

class TargetCell : public ui::custom::AutoWidgetCell
{
public:
	TargetCell(ui::Bitmap* bitmap, HostEnumerator* hostEnumerator, TargetInstance* instance);

	int32_t getHeight() const;

	virtual void placeCells(ui::custom::AutoWidget* widget, const ui::Rect& rect);

	virtual void paint(ui::custom::AutoWidget* widget, ui::Canvas& canvas, const ui::Rect& rect);

private:
	Ref< ProgressCell > m_progressCell;
	Ref< DropListCell > m_hostsCell;
	Ref< ButtonCell > m_playCell;
	Ref< TargetInstance > m_instance;
};

	}
}

#endif	// traktor_amalgam_TargetCell_H
