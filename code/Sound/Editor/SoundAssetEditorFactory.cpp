/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/SoundAssetEditor.h"
#include "Sound/Editor/SoundAssetEditorFactory.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundAssetEditorFactory", 0, SoundAssetEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet SoundAssetEditorFactory::getEditableTypes() const
{
	return makeTypeInfoSet< SoundAsset >();
}

bool SoundAssetEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return true;
}

Ref< editor::IObjectEditor > SoundAssetEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new SoundAssetEditor(editor);
}

void SoundAssetEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< ISerializable > SoundAssetEditorFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

}
