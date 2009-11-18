#include "Scene/Editor/DefaultEntityEditorFactory.h"
#include "Scene/Editor/DefaultEntityEditor.h"
#include "World/Entity/SpatialEntityData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.DefaultEntityEditorFactory", DefaultEntityEditorFactory, IEntityEditorFactory)

const TypeInfoSet DefaultEntityEditorFactory::getEntityDataTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< world::SpatialEntityData >());
	return typeSet;
}

Ref< IEntityEditor > DefaultEntityEditorFactory::createEntityEditor(
	SceneEditorContext* context,
	const TypeInfo& entityDataType
) const
{
	T_ASSERT (is_type_of< world::SpatialEntityData >(entityDataType));
	return new DefaultEntityEditor();
}

	}
}
