/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Input/Editor/InputMappingAsset.h"
#include "Input/Editor/InputMappingEditor.h"
#include "Input/Editor/InputMappingEditorFactory.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputMappingEditorFactory", 0, InputMappingEditorFactory, editor::IEditorPageFactory)

const TypeInfoSet InputMappingEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< InputMappingAsset >();
	return typeSet;
}

bool InputMappingEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IEditorPage > InputMappingEditorFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new InputMappingEditor(editor, site, document);
}

void InputMappingEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< ISerializable > InputMappingEditorFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

	}
}
