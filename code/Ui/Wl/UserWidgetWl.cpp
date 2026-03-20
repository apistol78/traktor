/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Wl/UserWidgetWl.h"

namespace traktor::ui
{

UserWidgetWl::UserWidgetWl(ContextWl* context, EventSubject* owner)
:	WidgetWlImpl< IUserWidget >(context, owner)
{
}

bool UserWidgetWl::create(IWidget* parent, int style)
{
	m_rect = Rect(0, 0, 0, 0);

	return WidgetWlImpl< IUserWidget >::create(
		parent,
		style,
		true,
		false
	);
}

}
