#ifndef traktor_script_IScriptManager_H
#define traktor_script_IScriptManager_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class IScriptClass;
class IScriptContext;
class IScriptResource;

/*! \brief Script error callback.
* \ingroup Script
*/
class IErrorCallback
{
public:
	virtual void syntaxError(uint32_t line, const std::wstring& message) = 0;

	virtual void otherError(const std::wstring& message) = 0;
};

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
	T_RTTI_CLASS;

public:
	/*! \brief Destroy script manager. */
	virtual void destroy() = 0;

	/*! \brief Register script class.
	 *
	 * \param scriptClass Script class interface.
	 */
	virtual void registerClass(IScriptClass* scriptClass) = 0;

	/*! \brief Compile script.
	 *
	 * \param script Script
	 * \param strip Strip debug information if available.
	 * \param errorCallback Optional callback which is called for each error found during compilation.
	 * \return Script resource; null if failed to compile.
	 */
	virtual Ref< IScriptResource > compile(const std::wstring& script, bool strip, IErrorCallback* errorCallback) const = 0;

	/*! \brief Create script context.
	 *
	 * \return Script context interface.
	 */
	virtual Ref< IScriptContext > createContext() = 0;
};

	}
}

#endif	// traktor_script_IScriptManager_H
