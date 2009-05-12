#ifndef traktor_spray_EffectEntityFactory_H
#define traktor_spray_EffectEntityFactory_H

#include "World/Entity/EntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

/*! \brief Effect entity factory.
 * \ingroup Spray
 */
class T_DLLCLASS EffectEntityFactory : public world::EntityFactory
{
	T_RTTI_CLASS(EffectEntityFactory)

public:
	virtual const TypeSet getEntityTypes() const;

	virtual world::Entity* createEntity(world::EntityBuilder* builder, const world::EntityData& entityData) const;
};

	}
}

#endif	// traktor_spray_EffectEntityFactory_H
