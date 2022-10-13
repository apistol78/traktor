#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Terrain/TerrainClassFactory.h"

namespace traktor::terrain
{

extern "C" void __module__Traktor_Terrain()
{
	T_FORCE_LINK_REF(TerrainClassFactory);
}

}

#endif
