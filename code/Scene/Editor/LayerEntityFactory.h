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
	virtual const TypeInfoSet getEntityTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getEntityEventTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getEntityComponentTypes() const T_OVERRIDE T_FINAL;

	virtual Ref< world::Entity > createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const T_OVERRIDE T_FINAL;

	virtual Ref< world::IEntityEvent > createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const T_OVERRIDE T_FINAL;

	virtual Ref< world::IEntityComponent > createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const T_OVERRIDE T_FINAL;
};
	
	}
}

#endif	// traktor_scene_LayerEntityFactory_H
