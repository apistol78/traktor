/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Animation/StateGraph.h"
#include "Animation/Editor/StateGraphEditorPage.h"
#include "Animation/Editor/StateGraphEditorPageFactory.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StateGraphEditorPageFactory", 0, StateGraphEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet StateGraphEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< StateGraph >());
	return typeSet;
}

bool StateGraphEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IEditorPage > StateGraphEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new StateGraphEditorPage(editor, site, document);
}

void StateGraphEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"StateGraph.Editor.AlignLeft"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.AlignRight"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.AlignTop"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.AlignBottom"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.EvenSpaceVertically"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.EventSpaceHorizontally"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.BrowseMesh"));
	outCommands.push_back(ui::Command(L"StateGraph.Editor.BrowseSkeleton"));
}

	}
}
