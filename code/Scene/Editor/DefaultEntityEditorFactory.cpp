#include "Scene/Editor/DefaultEntityEditorFactory.h"
#include "Scene/Editor/DefaultEntityEditor.h"
#include "World/Entity/SpatialEntityData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.DefaultEntityEditorFactory", DefaultEntityEditorFactory, IEntityEditorFactory)

const TypeSet DefaultEntityEditorFactory::getEntityDataTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< world::SpatialEntityData >());
	return typeSet;
}

IEntityEditor* DefaultEntityEditorFactory::createEntityEditor(
	SceneEditorContext* context,
	const Type& entityDataType
) const
{
	T_ASSERT (is_type_of< world::SpatialEntityData >(entityDataType));
	return gc_new< DefaultEntityEditor >();
}

	}
}
