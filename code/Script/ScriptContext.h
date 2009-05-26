#ifndef traktor_script_ScriptContext_H
#define traktor_script_ScriptContext_H

#include <vector>
#include "Core/Object.h"
#include "Script/Any.h"

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

/*! \brief Script error callback.
 * \ingroup Script
 */
class IErrorCallback
{
public:
	virtual void syntaxError(uint32_t line, const std::wstring& message) = 0;

	virtual void otherError(const std::wstring& message) = 0;
};

/*! \brief Script context.
 * \ingroup Script
 *
 * A script context is a thin, isolated, environment
 * in the script land.
 * They're created by the script manager and are
 * intended to be small environments which can
 * be embedded into entities etc.
 */
class T_DLLCLASS ScriptContext : public Object
{
	T_RTTI_CLASS(ScriptContext)

public:
	/*! \brief Set global variable value.
	 *
	 * \param globalName Name of global variable.
	 * \param globalValue Value of global variable.
	 */
	virtual void setGlobal(const std::wstring& globalName, const Any& globalValue) = 0;

	/*! \brief Execute script.
	 *
	 * \param script Script.
	 * \param compileOnly If script should only be compiled but not executed.
	 * \param errorCallback Optional error callback object.
	 * \return True if executed successfully.
	 */
	virtual bool executeScript(const std::wstring& script, bool compileOnly, IErrorCallback* errorCallback) = 0;

	/*! \brief Return true if context contains function.
	 *
	 * \param functionName Name of function of interest.
	 * \return True if function exists.
	 */
	virtual bool haveFunction(const std::wstring& functionName) const = 0;

	/*! \brief Execute function.
	 *
	 * \param functionName Name of function to execute.
	 * \param arguments Call arguments.
	 * \return Return value from function.
	 */
	virtual Any executeFunction(const std::wstring& functionName, const std::vector< Any >& arguments) = 0;
};

	}
}

#endif	// traktor_script_ScriptContext_H
