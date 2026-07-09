/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/RagDoll/RagDollSkeletonEditorFactory.h"

#include "Animation/Editor/RagDoll/RagDollSkeletonAsset.h"
#include "Animation/Editor/RagDoll/RagDollSkeletonEditor.h"
#include "Core/Serialization/DeepClone.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RagDollSkeletonEditorFactory", 0, RagDollSkeletonEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet RagDollSkeletonEditorFactory::getEditableTypes() const
{
	return makeTypeInfoSet< RagDollSkeletonAsset >();
}

bool RagDollSkeletonEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IObjectEditor > RagDollSkeletonEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new RagDollSkeletonEditor(editor);
}

void RagDollSkeletonEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< ISerializable > RagDollSkeletonEditorFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

}
