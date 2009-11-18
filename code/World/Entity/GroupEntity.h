#ifndef traktor_world_GroupEntity_H
#define traktor_world_GroupEntity_H

#include "Core/RefArray.h"
#include "World/Entity/Entity.h"

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

/*! \brief Group entity.
 * \ingroup World
 */
class T_DLLCLASS GroupEntity : public Entity
{
	T_RTTI_CLASS;

public:
	GroupEntity();

	virtual ~GroupEntity();

	virtual void destroy();

	void addEntity(Entity* entity);
	
	void removeEntity(Entity* entity);

	void removeAllEntities();
	
	const RefArray< Entity >& getEntities() const;
	
	int getEntitiesOf(const TypeInfo& entityType, RefArray< Entity >& outEntities) const;
	
	Ref< Entity > getFirstEntityOf(const TypeInfo& entityType) const;

	int getEntitiesOfRecursive(const TypeInfo& entityType, RefArray< Entity >& outEntities) const;
	
	Ref< Entity > getFirstEntityOfRecursive(const TypeInfo& entityType) const;

	virtual void update(const EntityUpdate* update);	

	/*! \name Template helpers. */
	//@{

	template < typename EntityType >
	int getEntitiesOf(RefArray< EntityType >& outEntities) const
	{
		return getEntitiesOf(
			type_of< EntityType >(),
			*reinterpret_cast< RefArray< Entity >* >(&outEntities)
		);
	}

	template < typename EntityType >
	Ref< EntityType > getFirstEntityOf() const
	{
		Ref< Entity > entity = getFirstEntityOf(type_of< EntityType >());
		return static_cast< EntityType* >(entity.ptr());
	}
	
	template < typename EntityType >
	int getEntitiesOfRecursive(RefArray< EntityType >& outEntities) const
	{
		return getEntitiesOfRecursive(
			type_of< EntityType >(),
			*reinterpret_cast< RefArray< Entity >* >(&outEntities)
		);
	}

	template < typename EntityType >
	Ref< EntityType > getFirstEntityOfRecursive() const
	{
		Ref< Entity > entity = getFirstEntityOfRecursive(type_of< EntityType >());
		return static_cast< EntityType* >(entity.ptr());
	}

	//@}

private:
	friend class GroupEntityFactory;

	RefArray< Entity > m_entities;
	bool m_update;
	RefArray< Entity > m_remove;
};
	
	}
}

#endif	// traktor_world_GroupEntity_H
