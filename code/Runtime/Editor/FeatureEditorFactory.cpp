/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Runtime/Editor/Deploy/Feature.h"
#include "Runtime/Editor/FeatureEditor.h"
#include "Runtime/Editor/FeatureEditorFactory.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.FeatureEditorFactory", 0, FeatureEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet FeatureEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< Feature >();
	return typeSet;
}

bool FeatureEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IObjectEditor > FeatureEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new FeatureEditor(editor);
}

void FeatureEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< ISerializable > FeatureEditorFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

	}
}
