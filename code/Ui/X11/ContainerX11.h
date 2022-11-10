/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/X11/WidgetX11Impl.h"
#include "Ui/Itf/IContainer.h"

namespace traktor
{
	namespace ui
	{

class ContainerX11 : public WidgetX11Impl< IContainer >
{
public:
	explicit ContainerX11(Context* context, EventSubject* owner);

	virtual bool create(IWidget* parent, int style) override final;
};

	}
}

