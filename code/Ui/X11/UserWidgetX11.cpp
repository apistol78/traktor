/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/X11/UserWidgetX11.h"

namespace traktor::ui
{

UserWidgetX11::UserWidgetX11(Context* context, EventSubject* owner)
:	WidgetX11Impl< IUserWidget >(context, owner)
{
}

bool UserWidgetX11::create(IWidget* parent, int style)
{
	WidgetData* parentData = static_cast< WidgetData* >(parent->getInternalHandle());

	Window window = XCreateWindow(
		m_context->getDisplay(),
		parentData->window,
		0,
		0,
		64,
		64,
		0,
		0,
		InputOutput,
		CopyFromParent,
		0,
		nullptr
	);

	return WidgetX11Impl< IUserWidget >::create(
		parent,
		style,
		window,
		Rect(0, 0, 0, 0),
		true,
		false
	);
}

}
