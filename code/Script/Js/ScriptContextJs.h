#ifndef traktor_script_ScriptContextJs_H
#define traktor_script_ScriptContextJs_H

#include <v8.h>
#define T_HAVE_TYPES
#include "Script/IScriptContext.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_JS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

struct lua_State;

namespace traktor
{
	namespace script
	{

class IScriptClass;

/*! \brief JavaScript scripting context.
 * \ingroup JS Script
 */
class T_DLLCLASS ScriptContextJs : public IScriptContext
{
	T_RTTI_CLASS(ScriptContextJs)

public:
	ScriptContextJs();

	virtual ~ScriptContextJs();

	bool create(const RefArray< IScriptClass >& registeredClasses);

	virtual void setGlobal(const std::wstring& globalName, const Any& globalValue);

	virtual Any getGlobal(const std::wstring& globalName);

	virtual bool executeScript(const std::wstring& script, bool compileOnly, IErrorCallback* errorCallback);

	virtual bool haveFunction(const std::wstring& functionName) const;

	virtual Any executeFunction(const std::wstring& functionName, const std::vector< Any >& arguments);

	virtual Any executeMethod(Object* self, const std::wstring& methodName, const std::vector< Any >& arguments);

private:
	struct RegisteredClass
	{
		Ref< IScriptClass > scriptClass;
		v8::Persistent< v8::FunctionTemplate > functionTemplate;
		v8::Persistent< v8::Function > function;
	};

	v8::Persistent< v8::Context > m_context;
	std::vector< RegisteredClass > m_classRegistry;

	static v8::Handle< v8::Value > invokeMethod(const v8::Arguments& arguments);

	static void weakHandleCallback(v8::Persistent< v8::Value > object, void* parameter);

	v8::Handle< v8::String > createString(const std::wstring& s) const;

	v8::Handle< v8::Object > createObject(Object* object) const;

	v8::Handle< v8::Value > toValue(const Any& value) const;

	Any fromValue(v8::Handle< v8::Value > value) const;
};

	}
}

#endif	// traktor_script_ScriptContextJs_H
