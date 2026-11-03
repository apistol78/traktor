/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Webber/ILayout.h"

namespace traktor::wbr
{

/*! Layout which splits the first two children horizontally or vertically.
 * \ingroup Webber
 */
class Splitter : public ILayout
{
	T_RTTI_CLASS;

public:
	enum Direction
	{
		Horizontal = 0,
		Vertical = 1
	};

	explicit Splitter(Direction direction);

	virtual void build(const Widget* widget, const RefArray< Widget >& children, html::Element* parent) const override final;

private:
	Direction m_direction;
};

}
