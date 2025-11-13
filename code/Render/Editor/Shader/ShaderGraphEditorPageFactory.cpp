/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphEditorPage.h"
#include "Render/Editor/Shader/ShaderGraphEditorPageFactory.h"
#include "Ui/Command.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderGraphEditorPageFactory", 0, ShaderGraphEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet ShaderGraphEditorPageFactory::getEditableTypes() const
{
	return makeTypeInfoSet< ShaderGraph >();
}

bool ShaderGraphEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IEditorPage > ShaderGraphEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new ShaderGraphEditorPage(editor, site, document);
}

void ShaderGraphEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.AlignBottom"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.AlignLeft"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.AlignRight"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.AlignTop"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.AutoMergeBranches"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.Center"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.CleanupSwizzles"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.ConstantFold"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.EvenSpaceHorizontally"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.EvenSpaceVertically"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.FindInDatabase"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.PlatformPermutation"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.QuickMenu"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.RemoveUnusedNodes"));
	outCommands.push_back(ui::Command(L"ShaderGraph.Editor.UpdateFragments"));
}

Ref< ISerializable > ShaderGraphEditorPageFactory::cloneAsset(const ISerializable* asset) const
{
	Ref< ShaderGraph > shaderGraph = DeepClone(asset).create< ShaderGraph >();
	if (!shaderGraph)
		return nullptr;

	for (auto node : shaderGraph->getNodes())
		node->setId(Guid::create());

	return shaderGraph;
}

}
