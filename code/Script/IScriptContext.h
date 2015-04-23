#ifndef traktor_script_IScriptContext_H
#define traktor_script_IScriptContext_H

#include <vector>
#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Class/Any.h"

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

/*! \brief Script context.
 * \ingroup Script
 *
 * A script context is a thin, isolated, environment
 * in the script land.
 * They're created by the script manager and are
 * intended to be small environments which can
 * be embedded into entities etc.
 */
class T_DLLCLASS IScriptContext : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Explicit destroy context. */
	virtual void destroy() = 0;

	/*! \brief Set global variable value.
	 *
	 * \param globalName Name of global variable.
	 * \param globalValue Value of global variable.
	 */
	virtual void setGlobal(const std::string& globalName, const Any& globalValue) = 0;

	/*! \brief Get global variable value.
	 *
	 * \param globalName Name of global variable.
	 * \return Global value; void if not found.
	 */
	virtual Any getGlobal(const std::string& globalName) = 0;

	/*! \brief Return true if context contains function (or method).
	 *
	 * \param functionName Name of function of interest.
	 * \return True if function exists.
	 */
	virtual bool haveFunction(const std::string& functionName) const = 0;

	/*! \brief Execute function.
	 *
	 * \param functionName Name of function to execute.
	 * \param argc Number of arguments.
	 * \param argv Argument vector.
	 * \return Return value from function.
	 */
	virtual Any executeFunction(const std::string& functionName, uint32_t argc = 0, const Any* argv = 0) = 0;

	/*! \brief Execute method.
	*
	* \param self This object.
	* \param methodName Name of method to execute.
	* \param argc Number of arguments.
	* \param argv Argument vector.
	* \return Return value from function.
	*/
	virtual Any executeMethod(Object* self, const std::string& methodName, uint32_t argc = 0, const Any* argv = 0) = 0;
};

	}
}

#endif	// traktor_script_IScriptContext_H
