#ifndef traktor_world_GroupEntity_H
#define traktor_world_GroupEntity_H

#include "Core/RefArray.h"
#include "World/Entity/Entity.h"

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

	virtual void destroy();

	void addEntity(Entity* entity);
	
	void removeEntity(Entity* entity);

	void removeAllEntities();
	
	const RefArray< Entity >& getEntities() const;
	
	int getEntitiesOf(const TypeInfo& entityType, RefArray< Entity >& outEntities) const;
	
	Ref< Entity > getFirstEntityOf(const TypeInfo& entityType) const;
	
	virtual void update(const EntityUpdate* update);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb3 getBoundingBox() const;

	template < typename EntityType >
	int getEntitiesOf(RefArray< EntityType >& outEntities) const
	{
		return getEntitiesOf(EntityType::getClassType(), *reinterpret_cast< RefArray< Entity >* >(&outEntities));
	}

	template < typename EntityType >
	Ref< EntityType > getFirstEntityOf() const
	{
		return static_cast< EntityType* >(getFirstEntityOf(EntityType::getClassType()));
	}
	
private:
	Transform m_transform;
	RefArray< Entity > m_entities;
	bool m_update;
	RefArray< Entity > m_remove;
};
	
	}
}

#endif	// traktor_world_GroupEntity_H
