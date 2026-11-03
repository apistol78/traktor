/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Webber/Widget.h"

namespace traktor::wbr
{

/*! HTML canvas element.
 * \ingroup Webber
 */
class Canvas : public Widget
{
	T_RTTI_CLASS;

public:
	virtual void build(html::Element* parent) const override final;
};

}
