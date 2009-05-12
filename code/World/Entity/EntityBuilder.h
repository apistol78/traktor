#ifndef traktor_world_EntitySetBuilder_H
#define traktor_world_EntitySetBuilder_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"

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

class EntityManager;
class EntityFactory;
class EntityData;
class Entity;

/*! \brief Entity builder.
 * \ingroup World
 *
 * Entity factory registration class; used
 * to select EntityFactory based on the type
 * of entity data given in order to build
 * a runtime entity.
 */
class T_DLLCLASS EntityBuilder : public Object
{
	T_RTTI_CLASS(EntityBuilder)

public:
	void addFactory(EntityFactory* entityFactory);

	void removeFactory(EntityFactory* entityFactory);

	void setEntityManager(EntityManager* entityManager);

	EntityManager* getEntityManager() const;

	Entity* build(const EntityData* entityData);

private:
	Ref< EntityManager > m_entityManager;
	RefArray< EntityFactory > m_entityFactories;
};

	}
}

#endif	// traktor_world_EntitySetBuilder_H
