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
	virtual void setGlobal(const std::wstring& globalName, const Any& globalValue) = 0;

	virtual bool executeScript(const std::wstring& script) = 0;

	virtual bool haveFunction(const std::wstring& functionName) const = 0;

	virtual Any executeFunction(const std::wstring& functionName, const std::vector< Any >& arguments) = 0;
};

	}
}

#endif	// traktor_script_ScriptContext_H
