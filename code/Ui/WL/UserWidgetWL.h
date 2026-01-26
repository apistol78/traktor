/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/WL/WidgetWLImpl.h"
#include "Ui/Itf/IUserWidget.h"

namespace traktor::ui
{

class UserWidgetWL : public WidgetWLImpl< IUserWidget >
{
public:
	explicit UserWidgetWL(Context* context, EventSubject* owner);

	virtual bool create(IWidget* parent, int style) override final;
};

}

