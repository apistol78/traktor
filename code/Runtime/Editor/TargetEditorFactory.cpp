/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Runtime/Editor/TargetEditor.h"
#include "Runtime/Editor/TargetEditorFactory.h"
#include "Runtime/Editor/Deploy/Target.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TargetEditorFactory", 0, TargetEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet TargetEditorFactory::getEditableTypes() const
{
	return makeTypeInfoSet< Target >();
}

bool TargetEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IObjectEditor > TargetEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new TargetEditor(editor);
}

void TargetEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< ISerializable > TargetEditorFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

}
