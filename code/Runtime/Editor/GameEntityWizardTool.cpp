/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Editor/GameEntityWizardTool.h"

#include "Core/Misc/SafeDestroy.h"
#include "I18N/Text.h"
#include "Runtime/Editor/GameEntityWizardDialog.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.GameEntityWizardTool", 0, GameEntityWizardTool, editor::IWizardTool)

bool GameEntityWizardTool::create(const PropertyGroup* settings)
{
	return true;
}

std::wstring GameEntityWizardTool::getDescription() const
{
	return i18n::Text(L"GAMEENTITY_WIZARD_DESCRIPTION");
}

const TypeInfoSet GameEntityWizardTool::getSupportedTypes() const
{
	return TypeInfoSet();
}

uint32_t GameEntityWizardTool::getFlags() const
{
	return WfGroup;
}

bool GameEntityWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	Ref< GameEntityWizardDialog > dialog = new GameEntityWizardDialog(editor, group);
	if (!dialog->create(parent))
		return false;

	dialog->showModal();

	safeDestroy(dialog);
	return true;
}

}
