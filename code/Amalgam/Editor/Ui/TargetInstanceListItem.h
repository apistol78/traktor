#ifndef traktor_amalgam_TargetInstanceListItem_H
#define traktor_amalgam_TargetInstanceListItem_H

#include "Amalgam/Editor/TargetInstance.h"
#include "Core/RefArray.h"
#include "Ui/Custom/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace amalgam
	{

class ButtonCell;
class DropListCell;
class HostEnumerator;
class ProgressCell;

class TargetInstanceListItem : public ui::custom::AutoWidgetCell
{
public:
	TargetInstanceListItem(HostEnumerator* hostEnumerator, TargetInstance* instance);

	ui::Size getSize() const;

	virtual void placeCells(ui::custom::AutoWidget* widget, const ui::Rect& rect);

	virtual void paint(ui::custom::AutoWidget* widget, ui::Canvas& canvas, const ui::Rect& rect);

private:
	Ref< ProgressCell > m_progressCell;
	Ref< DropListCell > m_hostsCell;
	Ref< ButtonCell > m_playCell;
	RefArray< ButtonCell > m_stopCells;
	RefArray< ButtonCell > m_captureCells;
	Ref< TargetInstance > m_instance;
	TargetState m_lastInstanceState;
};

	}
}

#endif	// traktor_amalgam_TargetInstanceListItem_H
