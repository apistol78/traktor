/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/AnimationGraph.h"
#include "Animation/Editor/AnimationGraphEditorPage.h"
#include "Animation/Editor/AnimationGraphEditorPageFactory.h"
#include "Core/Serialization/DeepClone.h"
#include "Ui/Command.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationGraphEditorPageFactory", 0, AnimationGraphEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet AnimationGraphEditorPageFactory::getEditableTypes() const
{
	return makeTypeInfoSet< AnimationGraph >();
}

bool AnimationGraphEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	outDependencies.insert(Guid(L"{84ADD065-E963-9D4D-A28D-FF44BD616B0F}"));
	return true;
}

Ref< editor::IEditorPage > AnimationGraphEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new AnimationGraphEditorPage(editor, site, document);
}

void AnimationGraphEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"AnimationGraph.Editor.AlignLeft"));
	outCommands.push_back(ui::Command(L"AnimationGraph.Editor.AlignRight"));
	outCommands.push_back(ui::Command(L"AnimationGraph.Editor.AlignTop"));
	outCommands.push_back(ui::Command(L"AnimationGraph.Editor.AlignBottom"));
	outCommands.push_back(ui::Command(L"AnimationGraph.Editor.EvenSpaceVertically"));
	outCommands.push_back(ui::Command(L"AnimationGraph.Editor.EventSpaceHorizontally"));
	outCommands.push_back(ui::Command(L"AnimationGraph.Editor.BrowseMesh"));
	outCommands.push_back(ui::Command(L"AnimationGraph.Editor.BrowseSkeleton"));
}

Ref< ISerializable > AnimationGraphEditorPageFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

}
