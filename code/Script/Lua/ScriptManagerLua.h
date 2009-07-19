#ifndef traktor_script_ScriptManagerLua_H
#define traktor_script_ScriptManagerLua_H

#include "Core/Heap/Ref.h"
#include "Script/IScriptManager.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_LUA_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

/*! \brief LUA script manager.
 * \ingroup LUA Script
 */
class T_DLLCLASS ScriptManagerLua : public IScriptManager
{
	T_RTTI_CLASS(ScriptManagerLua)

public:
	virtual void registerClass(IScriptClass* scriptClass);

	virtual IScriptClass* findScriptClass(const Type& type) const;

	virtual IScriptContext* createContext();

private:
	RefArray< IScriptClass > m_registeredClasses;
};

	}
}

#endif	// traktor_script_ScriptManagerLua_H
