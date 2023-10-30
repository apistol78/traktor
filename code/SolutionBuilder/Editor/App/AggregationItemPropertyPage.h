/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Container.h"
#include "Ui/Edit.h"

namespace traktor::sb
{

class AggregationItem;

class AggregationItemPropertyPage : public ui::Container
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	void set(AggregationItem* aggregationItem);

private:
	Ref< AggregationItem > m_aggregationItem;
	Ref< ui::Edit > m_editSourceFile;
	Ref< ui::Edit > m_editTargetPath;

	void eventEditFocus(ui::FocusEvent* event);
};

}
