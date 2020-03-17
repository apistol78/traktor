#include "Shape/Editor/Spline/SplineEntityData.h"
#include "Shape/Editor/Spline/SplineEntityEditor.h"
#include "Shape/Editor/Spline/SplineEntityEditorFactory.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SplineEntityEditorFactory", SplineEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet SplineEntityEditorFactory::getEntityDataTypes() const
{
	return makeTypeInfoSet< SplineEntityData >();
}

Ref< scene::IEntityEditor > SplineEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const 
{
	return new SplineEntityEditor(context, entityAdapter);
}

	}
}
