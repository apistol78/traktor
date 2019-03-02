#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Physics/Bullet/PhysicsManagerBullet.h"

namespace traktor
{
	namespace physics
	{

extern "C" void __module__Traktor_Physics_Bullet()
{
	T_FORCE_LINK_REF(PhysicsManagerBullet);
}

	}
}

#endif
