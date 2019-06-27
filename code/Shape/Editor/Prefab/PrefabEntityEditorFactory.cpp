#include "Shape/Editor/Prefab/PrefabEntityData.h"
#include "Shape/Editor/Prefab/PrefabEntityEditor.h"
#include "Shape/Editor/Prefab/PrefabEntityEditorFactory.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.PrefabEntityEditorFactory", PrefabEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet PrefabEntityEditorFactory::getEntityDataTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< PrefabEntityData >();
	return typeSet;
}

Ref< scene::IEntityEditor > PrefabEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const
{
	return new PrefabEntityEditor(context, entityAdapter);
}

	}
}
