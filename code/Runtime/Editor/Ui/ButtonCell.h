/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Command.h"
#include "Ui/IBitmap.h"
#include "Ui/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace runtime
	{

/*! \brief
 * \ingroup Runtime
 */
class ButtonCell : public ui::AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	ButtonCell(
		ui::IBitmap* bitmap,
		const ui::Command& command
	);

	void setEnable(bool enable);

	virtual void mouseDown(ui::MouseButtonDownEvent* event, const ui::Point& position)  override final;

	virtual void mouseUp(ui::MouseButtonUpEvent* event, const ui::Point& position)  override final;

	virtual void paint(ui::Canvas& canvas, const ui::Rect& rect)  override final;

private:
	Ref< ui::IBitmap > m_bitmap;
	ui::Command m_command;
	bool m_enable;
	bool m_down;
};

	}
}

