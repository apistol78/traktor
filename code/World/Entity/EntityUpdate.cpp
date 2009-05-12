#include "World/Entity/EntityUpdate.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityUpdate", EntityUpdate, Object)

EntityUpdate::EntityUpdate(float deltaTime)
:	m_deltaTime(deltaTime)
{
}

	}
}
