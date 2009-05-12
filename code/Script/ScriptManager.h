#ifndef traktor_script_ScriptManager_H
#define traktor_script_ScriptManager_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class ScriptClass;
class ScriptContext;

/*! \brief Script manager.
 * \ingroup Script
 *
 * A script manager is the major communicator
 * with the desired scripting environment.
 * It's purpose is to handle class registration
 * and context creations.
 * Each context should have at least the classes
 * registered up until the time of creation accessible.
 */
class T_DLLCLASS ScriptManager : public Object
{
	T_RTTI_CLASS(ScriptManager)

public:
	/*! \brief Register script class. */
	virtual void registerClass(ScriptClass* scriptClass) = 0;

	/*! \brief Create script context. */
	virtual ScriptContext* createContext() = 0;
};

	}
}

#endif	// traktor_script_ScriptManager_H
