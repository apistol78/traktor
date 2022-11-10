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

namespace traktor
{
	namespace sound
	{

class BankControlGrain;

class BankControl : public ui::AutoWidget
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void add(BankControlGrain* item);

	void removeAll();

	BankControlGrain* getSelected() const;

	const RefArray< BankControlGrain >& getGrains() const;

protected:
	virtual void layoutCells(const ui::Rect& rc) override final;

private:
	RefArray< BankControlGrain > m_cells;
};

	}
}

