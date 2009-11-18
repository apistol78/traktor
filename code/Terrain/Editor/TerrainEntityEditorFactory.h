#ifndef traktor_terrain_TerrainEntityEditorFactory_H
#define traktor_terrain_TerrainEntityEditorFactory_H

#include "Scene/Editor/IEntityEditorFactory.h"

namespace traktor
{
	namespace terrain
	{

/*! \brief
 */
class TerrainEntityEditorFactory : public scene::IEntityEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityDataTypes() const;

	virtual Ref< scene::IEntityEditor > createEntityEditor(scene::SceneEditorContext* context, const TypeInfo& entityDataType) const;
};

	}
}

#endif	// traktor_terrain_TerrainEntityEditorFactory_H
