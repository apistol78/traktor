/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "I18N/Text.h"
#include "Amalgam/Editor/GameEntityWizardDialog.h"
#include "Amalgam/Editor/GameEntityWizardTool.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.GameEntityWizardTool", 0, GameEntityWizardTool, editor::IWizardTool)

std::wstring GameEntityWizardTool::getDescription() const
{
	return i18n::Text(L"GAMEENTITY_WIZARD_DESCRIPTION");
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
}
