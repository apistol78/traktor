#include "Shape/Editor/Spline/ControlPointComponentEditor.h"
#include "Shape/Editor/Spline/ControlPointComponentEditorFactory.h"
#include "Shape/Spline/ControlPointComponentData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.ControlPointComponentEditorFactory", ControlPointComponentEditorFactory, scene::IComponentEditorFactory)

const TypeInfoSet ControlPointComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< ControlPointComponentData >();
}

Ref< scene::IComponentEditor > ControlPointComponentEditorFactory::createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new ControlPointComponentEditor(context, entityAdapter, componentData);
}

	}
}
