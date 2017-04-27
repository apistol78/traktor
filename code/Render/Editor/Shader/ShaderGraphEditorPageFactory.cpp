/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphEditorPage.h"
#include "Render/Editor/Shader/ShaderGraphEditorPageFactory.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderGraphEditorPageFactory", 0, ShaderGraphEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet ShaderGraphEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ShaderGraph >());
	return typeSet;
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

	}
}
