#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Script/Lua/ScriptManagerLua.h"

namespace traktor
{
	namespace script
	{

#	if defined(T_SCRIPT_LUAJIT_EXPORT)
extern "C" void __module__Traktor_Script_LuaJIT()
#	else
extern "C" void __module__Traktor_Script_Lua()
#	endif
{
	T_FORCE_LINK_REF(ScriptManagerLua);
}

	}
}

#endif
