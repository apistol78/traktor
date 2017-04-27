/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_SpawnEffectEvent_H
#define traktor_spray_SpawnEffectEvent_H

#include "Core/Math/Transform.h"
#include "Resource/Proxy.h"
#include "World/IEntityEvent.h"

namespace traktor
{
	namespace sound
	{

class ISoundPlayer;

	}

	namespace spray
	{

class Effect;

/*! \brief
 * \ingroup Spray
 */
class SpawnEffectEvent : public world::IEntityEvent
{
	T_RTTI_CLASS;

public:
	SpawnEffectEvent(
		sound::ISoundPlayer* soundPlayer,
		const resource::Proxy< Effect >& effect,
		const Transform& transform,
		bool follow,
		bool useRotation
	);

	virtual Ref< world::IEntityEventInstance > createInstance(world::IEntityEventManager* eventManager, world::Entity* sender, const Transform& Toffset) const T_OVERRIDE T_FINAL;

private:
	friend class SpawnEffectEventInstance;

	Ref< sound::ISoundPlayer > m_soundPlayer;
	resource::Proxy< Effect > m_effect;
	Transform m_transform;
	bool m_follow;
	bool m_useRotation;
};

	}
}

#endif	// traktor_spray_SpawnEffectEvent_H
