/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Runtime/Editor/TargetInstance.h"
#include "Ui/Auto/AutoWidgetCell.h"

namespace traktor::ui
{

class ButtonClickEvent;
class IBitmap;

}

namespace traktor::runtime
{

class ButtonCell;
class DropListCell;
class HostEnumerator;
class ProgressCell;

/*! Target instance list item.
 * \ingroup Runtime
 */
class TargetInstanceListItem : public ui::AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	explicit TargetInstanceListItem(HostEnumerator* hostEnumerator, TargetInstance* instance);

	ui::Size getSize() const;

	void setSelected(bool selected);

	virtual void placeCells(ui::AutoWidget* widget, const ui::Rect& rect) override final;

	virtual void paint(ui::Canvas& canvas, const ui::Rect& rect) override final;

private:
	Ref< ui::IBitmap > m_bitmapLogos;
	Ref< ProgressCell > m_progressCell;
	Ref< DropListCell > m_hostsCell;
	Ref< ButtonCell > m_playCell;
	Ref< ButtonCell > m_buildCell;
	Ref< ButtonCell > m_migrateCell;
	RefArray< ButtonCell > m_stopCells;
	RefArray< ButtonCell > m_captureCells;
	Ref< TargetInstance > m_instance;
	TargetState m_lastInstanceState;
	bool m_selected;

	void eventPlayButtonClick(ui::ButtonClickEvent* event);

	void eventBuildButtonClick(ui::ButtonClickEvent* event);

	void eventMigrateButtonClick(ui::ButtonClickEvent* event);

	void eventStopButtonClick(ui::ButtonClickEvent* event);

	void eventCaptureButtonClick(ui::ButtonClickEvent* event);
};

}
