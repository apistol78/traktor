/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef traktor_ui_UserWidgetX11_H
#define traktor_ui_UserWidgetX11_H

#include "Ui/X11/WidgetX11Impl.h"
#include "Ui/Itf/IUserWidget.h"

namespace traktor
{
	namespace ui
	{

class UserWidgetX11 : public WidgetX11Impl< IUserWidget >
{
public:
	explicit UserWidgetX11(Context* context, EventSubject* owner);

	virtual bool create(IWidget* parent, int style) override final;
};

	}
}

#endif	// traktor_ui_UserWidgetX11_H

