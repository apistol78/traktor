#ifndef traktor_spray_SpawnEffectEvent_H
#define traktor_spray_SpawnEffectEvent_H

#include "World/IEntityEvent.h"

namespace traktor
{
	namespace world
	{

class EntityData;
class IEntityBuilder;

	}

	namespace spray
	{

/*! \brief
 * \ingroup Spray
 */
class SpawnEffectEvent : public world::IEntityEvent
{
	T_RTTI_CLASS;

public:
	SpawnEffectEvent(
		const world::IEntityBuilder* entityBuilder,
		const world::EntityData* effectData,
		bool follow
	);

	virtual Ref< world::IEntityEventInstance > createInstance(world::Entity* sender, const Transform& Toffset) const;

private:
	Ref< const world::IEntityBuilder > m_entityBuilder;
	Ref< const world::EntityData > m_effectData;
	bool m_follow;
};

	}
}

#endif	// traktor_spray_SpawnEffectEvent_H
