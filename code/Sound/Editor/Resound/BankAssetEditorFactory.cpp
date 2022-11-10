/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Sound/Editor/Resound/BankAsset.h"
#include "Sound/Editor/Resound/BankAssetEditor.h"
#include "Sound/Editor/Resound/BankAssetEditorFactory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.BankAssetEditorFactory", 0, BankAssetEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet BankAssetEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< BankAsset >();
	return typeSet;
}

bool BankAssetEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return true;
}

Ref< editor::IObjectEditor > BankAssetEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new BankAssetEditor(editor);
}

void BankAssetEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< ISerializable > BankAssetEditorFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

	}
}
