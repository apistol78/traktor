/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "UiKit/Editor/PreviewEditor.h"
#include "UiKit/Editor/PreviewEditorFactory.h"
#include "UiKit/Editor/Scaffolding.h"

namespace traktor
{
	namespace uikit
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.uikit.PreviewEditorFactory", 0, PreviewEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet PreviewEditorFactory::getEditableTypes() const
{
	return makeTypeInfoSet< Scaffolding >();
}

bool PreviewEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	outDependencies.insert(Guid(L"{5B786C6B-8818-A24A-BD1C-EE113B79BCE2}"));	// System/Primitive/Shaders/Primitive
	outDependencies.insert(Guid(L"{123602E4-BC6F-874D-92E8-A20852D140A3}"));	// System/Primitive/Textures/SmallFont
	outDependencies.insert(Guid(L"{14D6A2DB-796D-E54D-9D70-73DE4AE7C4E8}"));	// System/Flash/Shaders/FlashShaderAssets
	return true;
}

Ref< editor::IObjectEditor > PreviewEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new PreviewEditor(editor);
}

void PreviewEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< ISerializable > PreviewEditorFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

	}
}
