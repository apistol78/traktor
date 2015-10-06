#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Script/ScriptClassFactory.h"

namespace traktor
{
	namespace script
	{

extern "C" void __module__Traktor_Script()
{
	T_FORCE_LINK_REF(ScriptClassFactory);
}

	}
}

#endif
