#include "Animation/Cloth/ClothComponentData.h"
#include "Animation/Editor/Cloth/ClothEntityEditor.h"
#include "Animation/Editor/Cloth/ClothEntityEditorFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.ClothEntityEditorFactory", ClothEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet ClothEntityEditorFactory::getEntityDataTypes() const
{
	return makeTypeInfoSet< ClothComponentData >();
}

Ref< scene::IEntityEditor > ClothEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const
{
	return new ClothEntityEditor(context, entityAdapter);
}

	}
}
