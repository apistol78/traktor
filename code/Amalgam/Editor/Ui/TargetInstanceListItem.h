/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_TargetInstanceListItem_H
#define traktor_amalgam_TargetInstanceListItem_H

#include "Amalgam/Editor/TargetInstance.h"
#include "Core/RefArray.h"
#include "Ui/Auto/ChildWidgetCell.h"

namespace traktor
{
	namespace ui
	{

class ButtonClickEvent;

	}

	namespace amalgam
	{

class ButtonCell;
class DropListCell;
class HostEnumerator;
class ProgressCell;

/*! \brief
 * \ingroup Amalgam
 */
class TargetInstanceListItem : public ui::AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	TargetInstanceListItem(HostEnumerator* hostEnumerator, TargetInstance* instance);

	ui::Size getSize() const;

	virtual void placeCells(ui::AutoWidget* widget, const ui::Rect& rect) override final;

	virtual void paint(ui::Canvas& canvas, const ui::Rect& rect) override final;

private:
	Ref< ProgressCell > m_progressCell;
	Ref< DropListCell > m_hostsCell;
	Ref< ButtonCell > m_playCell;
	Ref< ButtonCell > m_buildCell;
	Ref< ButtonCell > m_migrateCell;
	Ref< ButtonCell > m_browseCell;
	RefArray< ButtonCell > m_stopCells;
	RefArray< ButtonCell > m_captureCells;
	RefArray< ui::ChildWidgetCell > m_editCells;
	Ref< TargetInstance > m_instance;
	TargetState m_lastInstanceState;

	void eventPlayButtonClick(ui::ButtonClickEvent* event);

	void eventBuildButtonClick(ui::ButtonClickEvent* event);

	void eventMigrateButtonClick(ui::ButtonClickEvent* event);

	void eventBrowseButtonClick(ui::ButtonClickEvent* event);

	void eventStopButtonClick(ui::ButtonClickEvent* event);

	void eventCaptureButtonClick(ui::ButtonClickEvent* event);

	void eventCommandEditKeyDown(ui::KeyDownEvent* event);
};

	}
}

#endif	// traktor_amalgam_TargetInstanceListItem_H
