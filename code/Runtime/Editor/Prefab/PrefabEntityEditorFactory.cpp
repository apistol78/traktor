#include "Runtime/Editor/Prefab/PrefabEntityData.h"
#include "Runtime/Editor/Prefab/PrefabEntityEditor.h"
#include "Runtime/Editor/Prefab/PrefabEntityEditorFactory.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.PrefabEntityEditorFactory", PrefabEntityEditorFactory, scene::IEntityEditorFactory)

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
