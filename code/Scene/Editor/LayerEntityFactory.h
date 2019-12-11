#pragma once

#include "World/IEntityFactory.h"

namespace traktor
{
	namespace scene
	{

/*! Layer entity factory.
 * \ingroup Scene
 */
class LayerEntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityTypes() const override final;

	virtual const TypeInfoSet getEntityEventTypes() const override final;

	virtual const TypeInfoSet getEntityComponentTypes() const override final;

	virtual Ref< world::Entity > createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const override final;

	virtual Ref< world::IEntityEvent > createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const override final;

	virtual Ref< world::IEntityComponent > createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const override final;
};

	}
}

