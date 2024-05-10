/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Spray/EffectData.h"
#include "Spray/Editor/EffectEditorPage.h"
#include "Spray/Editor/EffectEditorPageFactory.h"
#include "Ui/Command.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EffectEditorPageFactory", 0, EffectEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet EffectEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< EffectData >();
	return typeSet;
}

bool EffectEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	outDependencies.insert(Guid(L"{5B786C6B-8818-A24A-BD1C-EE113B79BCE2}"));	// System/Primitive/Shaders/Primitive
	outDependencies.insert(Guid(L"{123602E4-BC6F-874D-92E8-A20852D140A3}"));	// System/Primitive/Textures/SmallFont
	outDependencies.insert(Guid(L"{84ADD065-E963-9D4D-A28D-FF44BD616B0F}"));
	return true;
}

Ref< editor::IEditorPage > EffectEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new EffectEditorPage(editor, site, document);
}

void EffectEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Effect.Editor.Rewind"));
	outCommands.push_back(ui::Command(L"Effect.Editor.Play"));
	outCommands.push_back(ui::Command(L"Effect.Editor.Stop"));
	outCommands.push_back(ui::Command(L"Effect.Editor.Reset"));
	outCommands.push_back(ui::Command(L"Effect.Editor.ToggleGuide"));
	outCommands.push_back(ui::Command(L"Effect.Editor.ToggleVelocity"));
	outCommands.push_back(ui::Command(L"Effect.Editor.ToggleGroundClip"));
	outCommands.push_back(ui::Command(L"Effect.Editor.RandomizeSeed"));
	outCommands.push_back(ui::Command(L"Effect.Editor.BrowseBackground"));
	outCommands.push_back(ui::Command(L"Effect.Editor.BrowseImageProcess"));
	outCommands.push_back(ui::Command(L"Effect.Editor.ReplaceEmitterSource"));
}

Ref< ISerializable > EffectEditorPageFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

}
