#include "Shape/Editor/Spline/ControlPointEntityEditor.h"
#include "Shape/Editor/Spline/ControlPointEntityEditorFactory.h"
#include "Shape/Spline/ControlPointEntityData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.ControlPointEntityEditorFactory", ControlPointEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet ControlPointEntityEditorFactory::getEntityDataTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ControlPointEntityData >());
	return typeSet;
}

Ref< scene::IEntityEditor > ControlPointEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const
{
	return new ControlPointEntityEditor(context, entityAdapter);
}

	}
}
