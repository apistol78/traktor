/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Ui/Auto/AutoWidget.h"

namespace traktor::runtime
{

class TargetInstanceListItem;

/*!
 * \ingroup Runtime
 */
class TargetListControl : public ui::AutoWidget
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void add(TargetInstanceListItem* item);

	void removeAll();

private:
	RefArray< TargetInstanceListItem > m_items;

	virtual void layoutCells(const ui::Rect& rc) override final;

	void eventButtonDown(ui::MouseButtonDownEvent* event);
};

}
