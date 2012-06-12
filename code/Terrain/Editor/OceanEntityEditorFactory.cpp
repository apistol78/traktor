#include "Scene/Editor/EntityAdapter.h"
#include "Terrain/OceanEntityData.h"
#include "Terrain/Editor/OceanEntityEditor.h"
#include "Terrain/Editor/OceanEntityEditorFactory.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.OceanEntityEditorFactory", OceanEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet OceanEntityEditorFactory::getEntityDataTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< OceanEntityData  >());
	return typeSet;
}

Ref< scene::IEntityEditor > OceanEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const
{
	const TypeInfo& entityDataType = type_of(entityAdapter->getEntityData());

	if (is_type_of< OceanEntityData >(entityDataType))
		return new OceanEntityEditor(context, entityAdapter);

	return 0;
}

	}
}
