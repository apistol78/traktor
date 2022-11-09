#if defined(T_STATIC)
#	include "World/WorldClassFactory.h"
#	include "World/Deferred/WorldRendererDeferred.h"
#	include "World/Forward/WorldRendererForward.h"
#	include "World/Simple/WorldRendererSimple.h"

namespace traktor::world
{

extern "C" void __module__Traktor_World()
{
	T_FORCE_LINK_REF(WorldClassFactory);
	T_FORCE_LINK_REF(WorldRendererDeferred);
	T_FORCE_LINK_REF(WorldRendererForward);
	T_FORCE_LINK_REF(WorldRendererSimple);
}

}
#endif
