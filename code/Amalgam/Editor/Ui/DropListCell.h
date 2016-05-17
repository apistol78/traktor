#ifndef traktor_amalgam_DropListCell_H
#define traktor_amalgam_DropListCell_H

#include "Ui/Command.h"
#include "Ui/Custom/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace amalgam
	{

class HostEnumerator;
class TargetInstance;

class DropListCell : public ui::custom::AutoWidgetCell
{
public:
	DropListCell(HostEnumerator* hostEnumerator, TargetInstance* instance);

protected:
	virtual void mouseDown(ui::MouseButtonDownEvent* event, const ui::Point& position)  T_OVERRIDE T_FINAL;

	virtual void paint(ui::Canvas& canvas, const ui::Rect& rect)  T_OVERRIDE T_FINAL;

private:
	Ref< HostEnumerator > m_hostEnumerator;
	Ref< TargetInstance > m_instance;
	ui::Point m_menuPosition;
};

	}
}

#endif	// traktor_amalgam_DropListCell_H
