/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Itf/IWidget.h"

namespace traktor::ui
{

/*! UserWidget interface.
 * \ingroup UI
 */
class IUserWidget : public IWidget
{
public:
	virtual bool create(IWidget* parent, int style) = 0;
};

}
