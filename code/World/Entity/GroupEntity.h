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

	virtual void destroy() T_OVERRIDE;

	void addEntity(Entity* entity);
	
	void removeEntity(Entity* entity);

	void removeAllEntities();
	
	const RefArray< Entity >& getEntities() const;
	
	int getEntitiesOf(const TypeInfo& entityType, RefArray< Entity >& outEntities) const;
	
	Ref< Entity > getFirstEntityOf(const TypeInfo& entityType) const;
	
	virtual void update(const UpdateParams& update) T_OVERRIDE;

	virtual void setTransform(const Transform& transform) T_OVERRIDE;

	virtual bool getTransform(Transform& outTransform) const T_OVERRIDE;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE;

	template < typename EntityType >
	int getEntitiesOf(RefArray< EntityType >& outEntities) const
	{
		return getEntitiesOf(type_of< EntityType >(), reinterpret_cast< RefArray< Entity >& >(outEntities));
	}

	template < typename EntityType >
	Ref< EntityType > getFirstEntityOf() const
	{
		Ref< Entity > entity = getFirstEntityOf(type_of< EntityType >());
		return reinterpret_cast< Ref< EntityType >& >(entity);
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
