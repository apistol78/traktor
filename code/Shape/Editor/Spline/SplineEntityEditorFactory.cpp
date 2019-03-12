#include "Shape/Editor/Spline/SplineEntityEditor.h"
#include "Shape/Editor/Spline/SplineEntityEditorFactory.h"
#include "Shape/Spline/SplineEntityData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SplineEntityEditorFactory", SplineEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet SplineEntityEditorFactory::getEntityDataTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< SplineEntityData >();
	return typeSet;
}

Ref< scene::IEntityEditor > SplineEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const
{
	return new SplineEntityEditor(context, entityAdapter);
}

	}
}
