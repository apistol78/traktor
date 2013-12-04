#ifndef traktor_scene_LayerEntityFactory_H
#define traktor_scene_LayerEntityFactory_H

#include "World/IEntityFactory.h"

namespace traktor
{
	namespace scene
	{

/*! \brief Layer entity factory.
 * \ingroup Scene
 */
class LayerEntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS;
	
public:
	virtual const TypeInfoSet getEntityTypes() const;

	virtual const TypeInfoSet getEntityEventTypes() const;

	virtual Ref< world::Entity > createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const;

	virtual Ref< world::IEntityEvent > createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const;
};
	
	}
}

#endif	// traktor_scene_LayerEntityFactory_H
