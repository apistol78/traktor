#ifndef traktor_script_ScriptManagerJs_H
#define traktor_script_ScriptManagerJs_H

#include "Core/Heap/Ref.h"
#include "Script/IScriptManager.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_JS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

/*! \brief JavaScript script manager.
 * \ingroup JS Script
 */
class T_DLLCLASS ScriptManagerJs : public IScriptManager
{
	T_RTTI_CLASS(ScriptManagerJs)

public:
	virtual void registerClass(IScriptClass* scriptClass);

	virtual IScriptContext* createContext();

private:
	RefArray< IScriptClass > m_registeredClasses;
};

	}
}

#endif	// traktor_script_ScriptManagerJs_H
