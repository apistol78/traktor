#pragma once

#include "Core/Ref.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace scene
	{

/*! External entity builder.
 *
 * This entity builder is used to filter non-visible entities
 * while loading entities from an ExternalEntity into scene editor.
 */
class ExternalEntityBuilder : public world::IEntityBuilder
{
	T_RTTI_CLASS;

public:
	ExternalEntityBuilder(world::IEntityBuilder* entityBuilder);

	virtual void addFactory(const world::IEntityFactory* entityFactory) override final;

	virtual void removeFactory(const world::IEntityFactory* entityFactory) override final;

	virtual const world::IEntityFactory* getFactory(const world::EntityData* entityData) const override final;

	virtual const world::IEntityFactory* getFactory(const world::IEntityEventData* entityEventData) const override final;

	virtual const world::IEntityFactory* getFactory(const world::IEntityComponentData* entityComponentData) const override final;

	virtual Ref< world::Entity > create(const world::EntityData* entityData) const override final;

	virtual Ref< world::IEntityEvent > create(const world::IEntityEventData* entityEventData) const override final;

	virtual Ref< world::IEntityComponent > create(const world::IEntityComponentData* entityComponentData) const override final;

	virtual const world::IEntityBuilder* getCompositeEntityBuilder() const override final;

private:
	Ref< world::IEntityBuilder > m_entityBuilder;
};

	}
}

