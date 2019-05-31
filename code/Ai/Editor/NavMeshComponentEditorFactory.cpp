#include "Ai/NavMeshComponentData.h"
#include "Ai/Editor/NavMeshComponentEditor.h"
#include "Ai/Editor/NavMeshComponentEditorFactory.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshComponentEditorFactory", NavMeshComponentEditorFactory, scene::IComponentEditorFactory)

const TypeInfoSet NavMeshComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< NavMeshComponentData >();
}

Ref< scene::IComponentEditor > NavMeshComponentEditorFactory::createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new NavMeshComponentEditor(context, entityAdapter, componentData);
}

	}
}
