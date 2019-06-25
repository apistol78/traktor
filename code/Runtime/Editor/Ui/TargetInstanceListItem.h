#pragma once

#include "Core/RefArray.h"
#include "Runtime/Editor/TargetInstance.h"
#include "Ui/Auto/ChildWidgetCell.h"

namespace traktor
{
	namespace ui
	{

class ButtonClickEvent;
class IBitmap;

	}

	namespace runtime
	{

class ButtonCell;
class DropListCell;
class HostEnumerator;
class ProgressCell;

/*! \brief
 * \ingroup Runtime
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
	Ref< ui::IBitmap > m_bitmapLogos;
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

