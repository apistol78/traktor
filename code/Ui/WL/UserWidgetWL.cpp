/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/WL/UserWidgetWL.h"

namespace traktor::ui
{

UserWidgetWL::UserWidgetWL(EventSubject* owner)
:	WidgetWLImpl< IUserWidget >(owner)
{
}

bool UserWidgetWL::create(IWidget* parent, int style)
{
	return true;
}

}
