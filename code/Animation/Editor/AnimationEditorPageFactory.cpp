/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Editor/AnimationEditorPageFactory.h"
#include "Animation/Editor/AnimationEditorPage.h"
#include "Animation/Animation/Animation.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationEditorPageFactory", 0, AnimationEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet AnimationEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Animation >());
	return typeSet;
}

bool AnimationEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	outDependencies.insert(Guid(L"{5B786C6B-8818-A24A-BD1C-EE113B79BCE2}"));	// System/Primitive/Shaders/Primitive
	outDependencies.insert(Guid(L"{123602E4-BC6F-874D-92E8-A20852D140A3}"));	// System/Primitive/Textures/SmallFont
	return false;
}

Ref< editor::IEditorPage > AnimationEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new AnimationEditorPage(editor, site, document);
}

void AnimationEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Animation.Editor.Rewind"));
	outCommands.push_back(ui::Command(L"Animation.Editor.Play"));
	outCommands.push_back(ui::Command(L"Animation.Editor.Stop"));
	outCommands.push_back(ui::Command(L"Animation.Editor.Forward"));
	outCommands.push_back(ui::Command(L"Animation.Editor.BrowseSkeleton"));
	outCommands.push_back(ui::Command(L"Animation.Editor.BrowseSkin"));
	outCommands.push_back(ui::Command(L"Animation.Editor.ToggleTrail"));
	outCommands.push_back(ui::Command(L"Animation.Editor.ToggleTwistLock"));
	outCommands.push_back(ui::Command(L"Animation.Editor.ToggleIK"));
	outCommands.push_back(ui::Command(L"Animation.Editor.SelectPreviousBone"));
	outCommands.push_back(ui::Command(L"Animation.Editor.SelectNextBone"));
}

	}
}
