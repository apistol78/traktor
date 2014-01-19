#include "Ai/NavMeshEntityData.h"
#include "Ai/Editor/NavMeshEntityEditor.h"
#include "Ai/Editor/NavMeshEntityEditorFactory.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshEntityEditorFactory", NavMeshEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet NavMeshEntityEditorFactory::getEntityDataTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< NavMeshEntityData >());
	return typeSet;
}

Ref< scene::IEntityEditor > NavMeshEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const
{
	return new NavMeshEntityEditor(context, entityAdapter);
}

	}
}
