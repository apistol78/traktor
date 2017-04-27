/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Script/Editor/Script.h"
#include "Script/Editor/ScriptAsset.h"
#include "Script/Editor/ScriptEditorPage.h"
#include "Script/Editor/ScriptEditorPageFactory.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptEditorPageFactory", 0, ScriptEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet ScriptEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Script >());
	typeSet.insert(&type_of< ScriptAsset >());
	return typeSet;
}

bool ScriptEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IEditorPage > ScriptEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new ScriptEditorPage(editor, site, document);
}

void ScriptEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Script.Editor.Continue"));
	outCommands.push_back(ui::Command(L"Script.Editor.Break"));
	outCommands.push_back(ui::Command(L"Script.Editor.StepInto"));
	outCommands.push_back(ui::Command(L"Script.Editor.StepOver"));
}

	}
}
