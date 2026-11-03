/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor::html
{

class Element;

}

namespace traktor::wbr
{

class Widget;

/*! Container layout; builds HTML around the container's children.
 * \ingroup Webber
 */
class ILayout : public Object
{
	T_RTTI_CLASS;

public:
	virtual void build(const Widget* widget, const RefArray< Widget >& children, html::Element* parent) const = 0;
};

}
