/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Illuminate/Editor/IlluminateEntityData.h"
#include "Illuminate/Editor/IlluminateEntityEditor.h"
#include "Illuminate/Editor/IlluminateEntityEditorFactory.h"

namespace traktor
{
	namespace illuminate
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.illuminate.IlluminateEntityEditorFactory", IlluminateEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet IlluminateEntityEditorFactory::getEntityDataTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< IlluminateEntityData >());
	return typeSet;
}

Ref< scene::IEntityEditor > IlluminateEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const
{
	return new IlluminateEntityEditor(context, entityAdapter);
}

	}
}
