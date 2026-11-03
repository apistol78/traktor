/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/Stack/Stack.h"
#include "Animation/Editor/Stack/StackEditorPage.h"
#include "Animation/Editor/Stack/StackEditorPageFactory.h"
#include "Core/Serialization/DeepClone.h"
#include "Ui/Command.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StackEditorPageFactory", 0, StackEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet StackEditorPageFactory::getEditableTypes() const
{
	return makeTypeInfoSet< Stack >();
}

bool StackEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	outDependencies.insert(Guid(L"{02ABDE03-E9F2-45A8-8FAA-3772EC7568ED}"));
	outDependencies.insert(Guid(L"{{5B786C6B-8818-A24A-BD1C-EE113B79BCE2}"));	// Primitive
	outDependencies.insert(Guid(L"{30D10D11-475A-C646-9433-23533E43EDAC}"));	// World assets
	return true;
}

Ref< editor::IEditorPage > StackEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new StackEditorPage(editor, site, document);
}

void StackEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Stack.Editor.BrowseMesh"));
	outCommands.push_back(ui::Command(L"Stack.Editor.BrowseSkeleton"));
}

Ref< ISerializable > StackEditorPageFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

}
