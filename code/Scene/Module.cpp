#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Scene/SceneClassFactory.h"

namespace traktor
{
	namespace scene
	{

extern "C" void __module__Traktor_Scene()
{
	T_FORCE_LINK_REF(SceneClassFactory);
}

	}
}

#endif
