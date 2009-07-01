#ifndef traktor_animation_PathEntityFactory_H
#define traktor_animation_PathEntityFactory_H

#include "World/Entity/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! \brief Movement path entity factory.
 * \ingroup Animation
 */
class T_DLLCLASS PathEntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS(PathEntityFactory)

public:
	virtual const TypeSet getEntityTypes() const;

	virtual world::Entity* createEntity(world::IEntityBuilder* builder, const std::wstring& name, const world::EntityData& entityData) const;
};

	}
}

#endif	// traktor_animation_PathEntityFactory_H
