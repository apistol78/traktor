#ifndef traktor_world_IEntityBuilder_H
#define traktor_world_IEntityBuilder_H

#include "Core/Object.h"
#include "Core/Heap/Ref.h"

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

class IEntityFactory;
class IEntityManager;
class EntityData;
class EntityInstance;
class Entity;

/*! \brief Entity builder interface.
 * \ingroup World
 */
class T_DLLCLASS IEntityBuilder : public Object
{
	T_RTTI_CLASS(IEntityBuilder)

public:
	virtual void addFactory(IEntityFactory* entityFactory) = 0;

	virtual void removeFactory(IEntityFactory* entityFactory) = 0;

	virtual void begin(IEntityManager* entityManager) = 0;

	virtual Ref< Entity > create(const std::wstring& name, const EntityData* entityData, const Object* instanceData) = 0;

	virtual Ref< Entity > build(const EntityInstance* instance) = 0;

	virtual void end() = 0;
};

	}
}

#endif	// traktor_world_IEntityBuilder_H
