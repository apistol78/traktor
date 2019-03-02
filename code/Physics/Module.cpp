#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Physics/PhysicsClassFactory.h"

namespace traktor
{
	namespace physics
	{

extern "C" void __module__Traktor_Physics()
{
	T_FORCE_LINK_REF(PhysicsClassFactory);
}

	}
}

#endif
