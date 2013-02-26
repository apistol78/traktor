#ifndef traktor_spray_SpawnEffectEventInstance_H
#define traktor_spray_SpawnEffectEventInstance_H

#include "Core/Math/Transform.h"
#include "World/IEntityEventInstance.h"

namespace traktor
{
	namespace world
	{

class Entity;

	}

	namespace spray
	{

class EffectEntity;

class SpawnEffectEventInstance : public world::IEntityEventInstance
{
	T_RTTI_CLASS;

public:
	SpawnEffectEventInstance(world::Entity* sender, const Transform& Toffset, EffectEntity* effectEntity, bool follow);

	virtual bool update(const world::UpdateParams& update);

	virtual void build(world::IWorldRenderer* worldRenderer);

private:
	Ref< world::Entity > m_sender;
	Transform m_Toffset;
	Ref< EffectEntity > m_effectEntity;
	bool m_follow;
};

	}
}

#endif	// traktor_spray_SpawnEffectEventInstance_H
