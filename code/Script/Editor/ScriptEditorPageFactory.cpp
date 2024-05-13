/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Script/Editor/Script.h"
#include "Script/Editor/ScriptEditorPage.h"
#include "Script/Editor/ScriptEditorPageFactory.h"
#include "Ui/Command.h"

namespace traktor::script
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptEditorPageFactory", 0, ScriptEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet ScriptEditorPageFactory::getEditableTypes() const
{
	return makeTypeInfoSet< Script >();
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

Ref< ISerializable > ScriptEditorPageFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

}
