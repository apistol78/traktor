/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Illuminate/Editor/IlluminateEntityData.h"
#include "Illuminate/Editor/IlluminateEntityEditor.h"
#include "Scene/Editor/EntityAdapter.h"

namespace traktor
{
	namespace illuminate
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.illuminate.IlluminateEntityEditor", IlluminateEntityEditor, scene::DefaultEntityEditor)

IlluminateEntityEditor::IlluminateEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
}

bool IlluminateEntityEditor::isPickable() const
{
	return false;
}

bool IlluminateEntityEditor::isGroup() const
{
	return true;
}

bool IlluminateEntityEditor::addChildEntity(traktor::scene::EntityAdapter* childEntityAdapter) const
{
	IlluminateEntityData* illumEntityData = checked_type_cast< IlluminateEntityData*, false >(getEntityAdapter()->getEntityData());
	
	world::EntityData* childEntityData = childEntityAdapter->getEntityData();
	illumEntityData->addEntityData(childEntityData);

	return true;
}

bool IlluminateEntityEditor::removeChildEntity(traktor::scene::EntityAdapter* childEntityAdapter) const
{
	IlluminateEntityData* illumEntityData = checked_type_cast< IlluminateEntityData*, false >(getEntityAdapter()->getEntityData());

	world::EntityData* childEntityData = childEntityAdapter->getEntityData();
	illumEntityData->removeEntityData(childEntityData);

	return true;
}

	}
}
