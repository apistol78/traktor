#include "Scene/Editor/EntityAdapter.h"
#include "Terrain/Editor/TerrainEntityEditorFactory.h"
#include "Terrain/Editor/TerrainEntityEditor.h"
#include "Terrain/Editor/OceanEntityEditor.h"
#include "Terrain/TerrainEntityData.h"
#include "Terrain/OceanEntityData.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEntityEditorFactory", TerrainEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet TerrainEntityEditorFactory::getEntityDataTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TerrainEntityData >());
	typeSet.insert(&type_of< OceanEntityData >());
	return typeSet;
}

Ref< scene::IEntityEditor > TerrainEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const
{
	const TypeInfo& entityDataType = type_of(entityAdapter->getEntityData());

	if (is_type_of< TerrainEntityData >(entityDataType))
		return TerrainEntityEditor::create(context, entityAdapter);
	else if (is_type_of< OceanEntityEditor >(entityDataType))
		return new OceanEntityEditor(context, entityAdapter);

	return 0;
}

	}
}
