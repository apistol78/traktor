#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)

#include "Runtime/GameClassFactory.h"
#include "Runtime/Engine/AudioLayerData.h"
#include "Runtime/Engine/RuntimePlugin.h"
#include "Runtime/Engine/VideoLayerData.h"
#include "Runtime/Engine/WorldLayerData.h"

namespace traktor
{
	namespace runtime
	{

extern "C" void __module__Traktor_Runtime()
{
	T_FORCE_LINK_REF(AudioLayerData);
	T_FORCE_LINK_REF(GameClassFactory);
	T_FORCE_LINK_REF(RuntimePlugin);
	T_FORCE_LINK_REF(VideoLayerData);
	T_FORCE_LINK_REF(WorldLayerData);
}

	}
}

#endif
