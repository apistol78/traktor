/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Dialog.h"
#include "Ui/WL/DialogWL.h"

namespace traktor::ui
{

DialogWL::DialogWL(Context* context, EventSubject* owner)
:	WidgetWLImpl< IDialog >(context, owner)
{
}

bool DialogWL::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	return true;
}

void DialogWL::setIcon(ISystemBitmap* icon)
{
}

DialogResult DialogWL::showModal()
{
	return DialogResult::Ok;
}

void DialogWL::endModal(DialogResult result)
{
}

void DialogWL::setMinSize(const Size& minSize)
{
}

}
