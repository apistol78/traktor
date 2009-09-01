#ifndef traktor_world_SpatialGroupEntity_H
#define traktor_world_SpatialGroupEntity_H

#include "Core/Heap/Ref.h"
#include "World/Entity/SpatialEntity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Group of spatial entities.
 * \ingroup World
 */
class T_DLLCLASS SpatialGroupEntity : public SpatialEntity
{
	T_RTTI_CLASS(SpatialGroupEntity)

public:
	SpatialGroupEntity(const Transform& transform);

	virtual ~SpatialGroupEntity();

	virtual void destroy();

	void addEntity(SpatialEntity* entity);
	
	void removeEntity(SpatialEntity* entity);

	void removeAllEntities();
	
	const RefArray< SpatialEntity >& getEntities() const;
	
	int getEntitiesOf(const Type& entityType, RefArray< SpatialEntity >& outEntities) const;
	
	SpatialEntity* getFirstEntityOf(const Type& entityType) const;
	
	virtual void update(const EntityUpdate* update);

	virtual void setTransform(const Transform& transform);

	virtual bool getTransform(Transform& outTransform) const;

	virtual Aabb getBoundingBox() const;

	template < typename EntityType >
	int getEntitiesOf(RefArray< EntityType >& outEntities) const
	{
		return getEntitiesOf(EntityType::getClassType(), *reinterpret_cast< RefArray< Entity >* >(&outEntities));
	}

	template < typename EntityType >
	EntityType* getFirstEntityOf() const
	{
		return static_cast< EntityType* >(getFirstEntityOf(EntityType::getClassType()));
	}
	
private:
	friend class GroupEntityFactory;

	Transform m_transform;
	RefArray< SpatialEntity > m_entities;
	bool m_update;
	RefArray< SpatialEntity > m_remove;
};
	
	}
}

#endif	// traktor_world_SpatialGroupEntity_H
