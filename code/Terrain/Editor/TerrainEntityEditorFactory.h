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
	T_RTTI_CLASS(TerrainEntityEditorFactory)

public:
	virtual const TypeSet getEntityDataTypes() const;

	virtual scene::IEntityEditor* createEntityEditor(scene::SceneEditorContext* context, const Type& entityDataType) const;
};

	}
}

#endif	// traktor_terrain_TerrainEntityEditorFactory_H
