#include "Spray/EffectEntity.h"
#include "Spray/SpawnEffectEventInstance.h"
#include "World/IWorldRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SpawnEffectEventInstance", SpawnEffectEventInstance, world::IEntityEventInstance)

SpawnEffectEventInstance::SpawnEffectEventInstance(world::Entity* sender, const Transform& Toffset, EffectEntity* effectEntity, bool follow)
:	m_sender(sender)
,	m_Toffset(Toffset)
,	m_effectEntity(effectEntity)
,	m_follow(follow)
{
	Transform T;
	m_sender->getTransform(T);
	m_effectEntity->setTransform(T * m_Toffset);
}

bool SpawnEffectEventInstance::update(const world::UpdateParams& update)
{
	if (m_follow)
	{
		Transform T;
		m_sender->getTransform(T);
		m_effectEntity->setTransform(T * m_Toffset);
	}

	m_effectEntity->update(update);

	return !m_effectEntity->isFinished();
}

void SpawnEffectEventInstance::build(world::IWorldRenderer* worldRenderer)
{
	worldRenderer->build(m_effectEntity);
}

	}
}
