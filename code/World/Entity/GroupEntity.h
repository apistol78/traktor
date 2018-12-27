/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_GroupEntity_H
#define traktor_world_GroupEntity_H

#include "Core/RefArray.h"
#include "World/Entity.h"

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

/*! \brief Group of entities.
 * \ingroup World
 */
class T_DLLCLASS GroupEntity : public Entity
{
	T_RTTI_CLASS;

public:
	GroupEntity(const Transform& transform = Transform::identity());

	virtual ~GroupEntity();

	virtual void destroy() override;

	void addEntity(Entity* entity);
	
	void removeEntity(Entity* entity);

	void removeAllEntities();
	
	const RefArray< Entity >& getEntities() const;
	
	int getEntitiesOf(const TypeInfo& entityType, RefArray< Entity >& outEntities) const;
	
	Entity* getFirstEntityOf(const TypeInfo& entityType) const;
	
	virtual void update(const UpdateParams& update) override;

	virtual void setTransform(const Transform& transform) override;

	virtual bool getTransform(Transform& outTransform) const override;

	virtual Aabb3 getBoundingBox() const override;

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
	Transform m_transform;
	RefArray< Entity > m_entities;
	bool m_update;
	RefArray< Entity > m_deferred[2];
};
	
	}
}

#endif	// traktor_world_GroupEntity_H
