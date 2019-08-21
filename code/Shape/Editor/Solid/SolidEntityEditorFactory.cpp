#include "Shape/Editor/Solid/SolidEntityData.h"
#include "Shape/Editor/Solid/SolidEntityEditor.h"
#include "Shape/Editor/Solid/SolidEntityEditorFactory.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SolidEntityEditorFactory", SolidEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet SolidEntityEditorFactory::getEntityDataTypes() const
{
	return makeTypeInfoSet< SolidEntityData >();
}

Ref< scene::IEntityEditor > SolidEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const
{
	return new SolidEntityEditor(context, entityAdapter);
}

	}
}
