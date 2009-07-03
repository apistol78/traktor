#ifndef traktor_script_IScriptManager_H
#define traktor_script_IScriptManager_H

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

class IScriptClass;
class IScriptContext;

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
class T_DLLCLASS IScriptManager : public Object
{
	T_RTTI_CLASS(IScriptManager)

public:
	/*! \brief Register script class. */
	virtual void registerClass(IScriptClass* scriptClass) = 0;

	/*! \brief Create script context. */
	virtual IScriptContext* createContext() = 0;
};

	}
}

#endif	// traktor_script_IScriptManager_H
