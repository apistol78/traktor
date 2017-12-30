#include "Amalgam/Editor/Prefab/PrefabEntityData.h"
#include "Amalgam/Editor/Prefab/PrefabEntityEditor.h"
#include "Scene/Editor/EntityAdapter.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.PrefabEntityEditor", PrefabEntityEditor, scene::DefaultEntityEditor)

PrefabEntityEditor::PrefabEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
}

bool PrefabEntityEditor::isPickable() const
{
	return false;
}

bool PrefabEntityEditor::isGroup() const
{
	return true;
}

bool PrefabEntityEditor::addChildEntity(traktor::scene::EntityAdapter* childEntityAdapter) const
{
	PrefabEntityData* prefabEntityData = checked_type_cast< PrefabEntityData* >(getEntityAdapter()->getEntityData());
	
	world::EntityData* childEntityData = childEntityAdapter->getEntityData();
	prefabEntityData->addEntityData(childEntityData);

	return true;
}

bool PrefabEntityEditor::removeChildEntity(traktor::scene::EntityAdapter* childEntityAdapter) const
{
	PrefabEntityData* prefabEntityData = checked_type_cast< PrefabEntityData* >(getEntityAdapter()->getEntityData());

	world::EntityData* childEntityData = childEntityAdapter->getEntityData();
	prefabEntityData->removeEntityData(childEntityData);

	return true;
}

	}
}
