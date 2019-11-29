#pragma once

#include "Core/RefArray.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;

/*! Group component.
 * \ingroup World
 */
class T_DLLCLASS GroupComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	GroupComponent();

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void update(const UpdateParams& update) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	void addEntity(Entity* entity);

	void removeEntity(Entity* entity);

	void removeAllEntities();

	const RefArray< Entity >& getEntities() const;

	int getEntitiesOf(const TypeInfo& entityType, RefArray< Entity >& outEntities) const;

	Entity* getFirstEntityOf(const TypeInfo& entityType) const;

	template < typename EntityType >
	int getEntitiesOf(RefArray< EntityType >& outEntities) const
	{
		return getEntitiesOf(type_of< EntityType >(), reinterpret_cast< RefArray< Entity >& >(outEntities));
	}

	template < typename EntityType >
	EntityType* getFirstEntityOf() const
	{
		Entity* entity = getFirstEntityOf(type_of< EntityType >());
		return reinterpret_cast< EntityType* >(entity);
	}

private:
	Entity* m_owner;
	Transform m_transform;
	RefArray< Entity > m_entities;
	bool m_update;
	RefArray< Entity > m_deferred[2];
};

	}
}
