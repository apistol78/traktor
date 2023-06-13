/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/App/EditorPageForm.h"
#include "Ui/FloodLayout.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.EditorPageForm", EditorPageForm, ui::Form)

bool EditorPageForm::create(ui::Widget* parent)
{
	if (!ui::Form::create(L"Editor Page", 1000_ut, 1000_ut, ui::Form::WsDefault, new ui::FloodLayout()))
		return false;

	return true;
}

void EditorPageForm::destroy()
{
	ui::Form::destroy();
}

	}
}
