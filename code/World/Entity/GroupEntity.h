#ifndef traktor_world_GroupEntity_H
#define traktor_world_GroupEntity_H

#include "Core/Heap/Ref.h"
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
	T_RTTI_CLASS(GroupEntity)

public:
	GroupEntity();

	virtual ~GroupEntity();

	virtual void destroy();

	void addEntity(Entity* entity);
	
	void removeEntity(Entity* entity);

	void removeAllEntities();
	
	const RefArray< Entity >& getEntities() const;
	
	int getEntitiesOf(const Type& entityType, RefArray< Entity >& outEntities) const;
	
	Entity* getFirstEntityOf(const Type& entityType) const;

	int getEntitiesOfRecursive(const Type& entityType, RefArray< Entity >& outEntities) const;
	
	Entity* getFirstEntityOfRecursive(const Type& entityType) const;

	/*! \brief Find named child group.
	 *
	 * Iterate through it's children to find a named
	 * child group.
	 * Do NOT use this frequently as it relies on rtti
	 * types to investigate children entities and thus
	 * are really slow.
	 */
	GroupEntity* findChildGroup(const std::wstring& name) const;

	/*! \brief Find named child group recursively.
	 *
	 * Iterate through it's children to find a named
	 * child group.
	 * Do NOT use this frequently as it relies on rtti
	 * types to investigate children entities and thus
	 * are really slow.
	 */
	GroupEntity* findChildGroupRecursive(const std::wstring& name) const;
	
	virtual void update(const EntityUpdate* update);	

	/*! \name Template helpers. */
	//@{

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
	
	template < typename EntityType >
	int getEntitiesOfRecursive(RefArray< EntityType >& outEntities) const
	{
		return getEntitiesOfRecursive(EntityType::getClassType(), *reinterpret_cast< RefArray< Entity >* >(&outEntities));
	}

	template < typename EntityType >
	EntityType* getFirstEntityOfRecursive() const
	{
		return static_cast< EntityType* >(getFirstEntityOfRecursive(EntityType::getClassType()));
	}

	//@}

private:
	friend class GroupEntityFactory;

	std::wstring m_name;
	RefArray< Entity > m_entities;
	bool m_update;
	RefArray< Entity > m_remove;
};
	
	}
}

#endif	// traktor_world_GroupEntity_H
