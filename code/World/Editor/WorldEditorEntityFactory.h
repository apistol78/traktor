#pragma once

#include "World/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*!
 * \ingroup World
 */
class T_DLLCLASS WorldEditorEntityFactory : public IEntityFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityTypes() const override final;

	virtual const TypeInfoSet getEntityEventTypes() const override final;

	virtual const TypeInfoSet getEntityComponentTypes() const override final;

	virtual Ref< Entity > createEntity(const IEntityBuilder* builder, const EntityData& entityData) const override final;

	virtual Ref< IEntityEvent > createEntityEvent(const IEntityBuilder* builder, const IEntityEventData& entityEventData) const override final;

	virtual Ref< IEntityComponent > createEntityComponent(const world::IEntityBuilder* builder, const IEntityComponentData& entityComponentData) const override final;
};

	}
}

