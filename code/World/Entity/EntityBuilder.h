#ifndef traktor_world_EntitySetBuilder_H
#define traktor_world_EntitySetBuilder_H

#include <map>
#include "Core/Heap/Ref.h"
#include "World/Entity/IEntityBuilder.h"

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

/*! \brief Entity builder.
 * \ingroup World
 */
class T_DLLCLASS EntityBuilder : public IEntityBuilder
{
	T_RTTI_CLASS(EntityBuilder)

public:
	virtual void addFactory(IEntityFactory* entityFactory);

	virtual void removeFactory(IEntityFactory* entityFactory);

	virtual void begin(IEntityManager* entityManager);

	virtual Entity* create(const std::wstring& name, const EntityData* entityData, const Object* instanceData);

	virtual Entity* build(const EntityInstance* instance);

	virtual void end();

private:
	Ref< IEntityManager > m_entityManager;
	RefArray< IEntityFactory > m_entityFactories;
	std::map< const EntityInstance*, Ref< Entity > > m_instances;
};

	}
}

#endif	// traktor_world_EntitySetBuilder_H
