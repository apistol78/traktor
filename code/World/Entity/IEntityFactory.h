#ifndef traktor_world_IEntityFactory_H
#define traktor_world_IEntityFactory_H

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

class IEntityBuilder;
class EntityData;
class Entity;

/*! \brief Entity factory interface.
 * \ingroup World
 */
class T_DLLCLASS IEntityFactory : public Object
{
	T_RTTI_CLASS(IEntityFactory)

public:
	virtual const TypeSet getEntityTypes() const = 0;

	virtual Ref< Entity > createEntity(
		IEntityBuilder* builder,
		const std::wstring& name,
		const EntityData& entityData,
		const Object* instanceData
	) const = 0;
};

	}
}

#endif	// traktor_world_IEntityFactory_H
