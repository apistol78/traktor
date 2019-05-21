#include "Animation/AnimatedMeshComponentData.h"
#include "Animation/Editor/AnimatedMeshComponentEditor.h"
#include "Animation/Editor/AnimatedMeshComponentEditorFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshComponentEditorFactory", AnimatedMeshComponentEditorFactory, scene::IComponentEditorFactory)

const TypeInfoSet AnimatedMeshComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< AnimatedMeshComponentData >();
}

Ref< scene::IComponentEditor > AnimatedMeshComponentEditorFactory::createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new AnimatedMeshComponentEditor(context, entityAdapter, componentData);
}

	}
}
