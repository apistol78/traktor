/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/AnimatedMeshEntityData.h"
#include "Animation/Cloth/ClothEntityData.h"
#include "Animation/Editor/AnimatedMeshEntityEditor.h"
#include "Animation/Editor/AnimationEntityEditorFactory.h"
#include "Animation/Editor/Cloth/ClothEntityEditor.h"
#include "Animation/Editor/PathEntity/PathEntityEditor.h"
#include "Animation/PathEntity/PathEntityData.h"
#include "Scene/Editor/EntityAdapter.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationEntityEditorFactory", AnimationEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet AnimationEntityEditorFactory::getEntityDataTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< AnimatedMeshEntityData >());
	typeSet.insert(&type_of< ClothEntityData >());
	typeSet.insert(&type_of< PathEntityData >());
	return typeSet;
}

Ref< scene::IEntityEditor > AnimationEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const
{
	const TypeInfo& entityDataType = type_of(entityAdapter->getEntityData());

	if (is_type_of< AnimatedMeshEntityData >(entityDataType))
		return new AnimatedMeshEntityEditor(context, entityAdapter);
	else if (is_type_of< ClothEntityData >(entityDataType))
		return new ClothEntityEditor(context, entityAdapter);
	else if (is_type_of< PathEntityData >(entityDataType))
		return new PathEntityEditor(context, entityAdapter);

	return 0;
}

	}
}
