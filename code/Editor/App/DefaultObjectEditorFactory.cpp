/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Editor/App/DefaultObjectEditor.h"
#include "Editor/App/DefaultObjectEditorFactory.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.DefaultObjectEditorFactory", 0, DefaultObjectEditorFactory, IObjectEditorFactory)

const TypeInfoSet DefaultObjectEditorFactory::getEditableTypes() const
{
	return makeTypeInfoSet< ISerializable >();
}

bool DefaultObjectEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< IObjectEditor > DefaultObjectEditorFactory::createObjectEditor(IEditor* editor) const
{
	return new DefaultObjectEditor(editor);
}

void DefaultObjectEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< ISerializable > DefaultObjectEditorFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

	}
}
