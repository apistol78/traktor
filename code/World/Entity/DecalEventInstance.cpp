#include "Core/Misc/SafeDestroy.h"
#include "World/IWorldRenderer.h"
#include "World/Entity/DecalEntity.h"
#include "World/Entity/DecalEvent.h"
#include "World/Entity/DecalEventInstance.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DecalEventInstance", DecalEventInstance, IEntityEventInstance)

DecalEventInstance::DecalEventInstance(const DecalEvent* event, const Transform& Toffset)
{
	m_entity = new DecalEntity(
		Toffset,
		event->getSize(),
		event->getThickness(),
		event->getAlpha(),
		event->getCullDistance(),
		event->getShader()
	);
}

bool DecalEventInstance::update(const UpdateParams& update)
{
	if (m_entity)
	{
		m_entity->update(update);
		if (m_entity->getAlpha() > FUZZY_EPSILON)
			return true;
	}
	return false;
}

void DecalEventInstance::build(IWorldRenderer* worldRenderer)
{
	worldRenderer->build(m_entity);
}

void DecalEventInstance::cancel()
{
	safeDestroy(m_entity);
}

	}
}
