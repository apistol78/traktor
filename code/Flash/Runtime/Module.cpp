#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Flash/Runtime/FlashClassFactory.h"
#	include "Flash/Runtime/FlashLayerData.h"
#	include "Flash/Runtime/RuntimePlugin.h"

namespace traktor
{
	namespace flash
	{

extern "C" void __module__Traktor_Flash_Runtime()
{
	T_FORCE_LINK_REF(FlashClassFactory);
	T_FORCE_LINK_REF(FlashLayerData);
	T_FORCE_LINK_REF(RuntimePlugin);
}

	}
}

#endif
