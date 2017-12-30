#include "Amalgam/Editor/Prefab/PrefabEntityData.h"
#include "Amalgam/Editor/Prefab/PrefabEntityEditor.h"
#include "Amalgam/Editor/Prefab/PrefabEntityEditorFactory.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.PrefabEntityEditorFactory", PrefabEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet PrefabEntityEditorFactory::getEntityDataTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PrefabEntityData >());
	return typeSet;
}

Ref< scene::IEntityEditor > PrefabEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const
{
	return new PrefabEntityEditor(context, entityAdapter);
}

	}
}
