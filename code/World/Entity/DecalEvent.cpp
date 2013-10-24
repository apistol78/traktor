#include "World/Entity/DecalEvent.h"
#include "World/Entity/DecalEventInstance.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DecalEvent", DecalEvent, IEntityEvent)

DecalEvent::DecalEvent()
:	m_size(0.0f)
,	m_thickness(0.0f)
,	m_alpha(0.0f)
,	m_cullDistance(0.0f)
{
}

Ref< IEntityEventInstance > DecalEvent::createInstance(IEntityEventManager* eventManager, Entity* sender, const Transform& Toffset) const
{
	return new DecalEventInstance(this, Toffset);
}

	}
}
