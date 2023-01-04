/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Render/Editor/SH/SHEditorPage.h"
#include "Render/Editor/SH/SHEditorPageFactory.h"
#include "Render/Editor/SH/SHStage.h"
#include "Ui/Command.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SHEditorPageFactory", 0, SHEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet SHEditorPageFactory::getEditableTypes() const
{
	return makeTypeInfoSet< SHStage >();
}

bool SHEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	outDependencies.insert(Guid(L"{5B786C6B-8818-A24A-BD1C-EE113B79BCE2}"));	// System/Primitive/Shaders/Primitive
	outDependencies.insert(Guid(L"{123602E4-BC6F-874D-92E8-A20852D140A3}"));	// System/Primitive/Textures/SmallFont
	return false;
}

Ref< editor::IEditorPage > SHEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new  SHEditorPage(editor);
}

void SHEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"SHEditor.Editor.Randomize"));
}

Ref< ISerializable > SHEditorPageFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

}
