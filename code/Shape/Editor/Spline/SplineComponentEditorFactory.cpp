#include "Shape/Editor/Spline/SplineComponentEditor.h"
#include "Shape/Editor/Spline/SplineComponentEditorFactory.h"
#include "Shape/Editor/Spline/SplineComponentData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SplineComponentEditorFactory", SplineComponentEditorFactory, scene::IComponentEditorFactory)

const TypeInfoSet SplineComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< SplineComponentData >();
}

Ref< scene::IComponentEditor > SplineComponentEditorFactory::createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new SplineComponentEditor(context, entityAdapter, componentData);
}

	}
}
