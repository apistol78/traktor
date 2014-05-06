#include "Core/Misc/SafeDestroy.h"
#include "Spray/EffectEntity.h"
#include "Spray/SpawnEffectEvent.h"
#include "Spray/SpawnEffectEventInstance.h"
#include "World/IWorldRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SpawnEffectEventInstance", SpawnEffectEventInstance, world::IEntityEventInstance)

SpawnEffectEventInstance::SpawnEffectEventInstance(const SpawnEffectEvent* spawnEffect, world::Entity* sender, const Transform& Toffset, EffectEntity* effectEntity)
:	m_spawnEffect(spawnEffect)
,	m_sender(sender)
,	m_Toffset(Toffset)
,	m_effectEntity(effectEntity)
{
	Transform T;
	if (m_sender)
	{
		m_sender->getTransform(T);
		T = T * m_Toffset;
	}
	else
		T = m_Toffset;

	if (m_spawnEffect->m_useRotation)
		m_effectEntity->setTransform(T);
	else
		m_effectEntity->setTransform(Transform(T.translation()));
}

bool SpawnEffectEventInstance::update(const world::UpdateParams& update)
{
	if (!m_effectEntity)
		return false;

	if (m_spawnEffect->m_follow)
	{
		Transform T;
		if (m_sender)
		{
			m_sender->getTransform(T);
			T = T * m_Toffset;
		}

		if (m_spawnEffect->m_useRotation)
			m_effectEntity->setTransform(T);
		else
			m_effectEntity->setTransform(Transform(T.translation()));
	}

	m_effectEntity->update(update);

	return !m_effectEntity->isFinished();
}

void SpawnEffectEventInstance::build(world::IWorldRenderer* worldRenderer)
{
	worldRenderer->build(m_effectEntity);
}

void SpawnEffectEventInstance::cancel(CancelType when)
{
	if (when == IEntityEventInstance::CtImmediate)
		safeDestroy(m_effectEntity);
	else
	{
		if (m_effectEntity)
			m_effectEntity->setLoopEnable(false);
	}
}

	}
}
