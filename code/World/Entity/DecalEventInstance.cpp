/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "World/IWorldRenderer.h"
#include "World/Entity/ComponentEntity.h"
#include "World/Entity/DecalComponent.h"
#include "World/Entity/DecalEvent.h"
#include "World/Entity/DecalEventInstance.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DecalEventInstance", DecalEventInstance, IEntityEventInstance)

DecalEventInstance::DecalEventInstance(const DecalEvent* event, const Transform& Toffset)
{
	m_entity = new ComponentEntity();
	m_entity->setComponent(new DecalComponent(
		event->getSize(),
		event->getThickness(),
		event->getAlpha(),
		event->getCullDistance(),
		event->getShader()
	));
	m_entity->setTransform(Toffset);
}

bool DecalEventInstance::update(const UpdateParams& update)
{
	if (m_entity)
	{
		m_entity->update(update);
		if (m_entity->getComponent< DecalComponent >()->getAlpha() > FUZZY_EPSILON)
			return true;
	}
	return false;
}

void DecalEventInstance::build(IWorldRenderer* worldRenderer)
{
	worldRenderer->build(m_entity);
}

void DecalEventInstance::cancel(CancelType when)
{
	safeDestroy(m_entity);
}

	}
}
