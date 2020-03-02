#include "Animation/PathEntity/PathComponentData.h"
#include "Animation/Editor/PathEntity/PathComponentEditor.h"
#include "Animation/Editor/PathEntity/PathComponentEditorFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathComponentEditorFactory", PathComponentEditorFactory, scene::IComponentEditorFactory)

const TypeInfoSet PathComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< PathComponentData >();
}

Ref< scene::IComponentEditor > PathComponentEditorFactory::createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new PathComponentEditor(context, entityAdapter, componentData);
}

	}
}
