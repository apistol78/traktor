#ifndef traktor_world_EntitySetBuilder_H
#define traktor_world_EntitySetBuilder_H

#include <map>
#include "Core/RefArray.h"
#include "World/Entity/IEntityBuilder.h"

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

/*! \brief Entity builder.
 * \ingroup World
 */
class T_DLLCLASS EntityBuilder : public IEntityBuilder
{
	T_RTTI_CLASS;

public:
	EntityBuilder();

	virtual void addFactory(IEntityFactory* entityFactory);

	virtual void removeFactory(IEntityFactory* entityFactory);

	virtual void begin(IEntityManager* entityManager);

	virtual Ref< Entity > create(const EntityData* entityData);

	virtual Ref< Entity > get(const EntityData* entityData) const;

	virtual void end();

private:
	Ref< IEntityManager > m_entityManager;
	RefArray< IEntityFactory > m_entityFactories;
	std::map< const EntityData*, Ref< Entity > > m_entities;
	bool m_inbuild;
};

	}
}

#endif	// traktor_world_EntitySetBuilder_H
