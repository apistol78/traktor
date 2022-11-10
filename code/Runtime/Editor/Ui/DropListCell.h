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
#include "Ui/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace runtime
	{

class HostEnumerator;
class TargetInstance;

/*! \brief
 * \ingroup Runtime
 */
class DropListCell : public ui::AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	DropListCell(HostEnumerator* hostEnumerator, TargetInstance* instance);

protected:
	virtual void mouseDown(ui::MouseButtonDownEvent* event, const ui::Point& position)  override final;

	virtual void paint(ui::Canvas& canvas, const ui::Rect& rect)  override final;

private:
	Ref< HostEnumerator > m_hostEnumerator;
	Ref< TargetInstance > m_instance;
	ui::Point m_menuPosition;
};

	}
}

