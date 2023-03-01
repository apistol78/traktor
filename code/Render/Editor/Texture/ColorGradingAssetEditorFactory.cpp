/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Render/Editor/Texture/ColorGradingAssetEditor.h"
#include "Render/Editor/Texture/ColorGradingAssetEditorFactory.h"
#include "Render/Editor/Texture/ColorGradingTextureAsset.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.ColorGradingAssetEditorFactory", 0, ColorGradingAssetEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet ColorGradingAssetEditorFactory::getEditableTypes() const
{
	return makeTypeInfoSet< ColorGradingTextureAsset >();
}

bool ColorGradingAssetEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return true;
}

Ref< editor::IObjectEditor > ColorGradingAssetEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new ColorGradingAssetEditor(editor);
}

void ColorGradingAssetEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< ISerializable > ColorGradingAssetEditorFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

}
