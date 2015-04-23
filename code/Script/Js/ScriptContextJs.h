#ifndef traktor_script_ScriptContextJs_H
#define traktor_script_ScriptContextJs_H

#include <v8.h>
#define T_HAVE_TYPES
#include "Core/RefArray.h"
#include "Script/IScriptContext.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_JS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

struct lua_State;

namespace traktor
{

class IRuntimeClass;

	namespace script
	{

class IScriptResource;

/*! \brief JavaScript scripting context.
 * \ingroup JS Script
 */
class T_DLLCLASS ScriptContextJs : public IScriptContext
{
	T_RTTI_CLASS;

public:
	ScriptContextJs();

	virtual ~ScriptContextJs();

	bool create(const RefArray< IRuntimeClass >& registeredClasses, const IScriptResource* scriptResource);

	virtual void destroy();

	virtual void setGlobal(const std::string& globalName, const Any& globalValue);

	virtual Any getGlobal(const std::string& globalName);

	virtual bool haveFunction(const std::string& functionName) const;

	virtual Any executeFunction(const std::string& functionName, uint32_t argc, const Any* argv);

	virtual Any executeMethod(Object* self, const std::string& methodName, uint32_t argc, const Any* argv);

private:
	struct RegisteredClass
	{
		Ref< IRuntimeClass > runtimeClass;
		v8::Persistent< v8::FunctionTemplate > functionTemplate;
		v8::Persistent< v8::Function > function;
	};

	v8::Persistent< v8::Context > m_context;
	std::vector< RegisteredClass > m_classRegistry;

	static v8::Handle< v8::Value > invokeConstructor(const v8::Arguments& arguments);

	static v8::Handle< v8::Value > invokeMethod(const v8::Arguments& arguments);

	static void weakHandleCallback(v8::Persistent< v8::Value > object, void* parameter);

	v8::Handle< v8::String > createString(const std::string& s) const;

	v8::Handle< v8::String > createString(const std::wstring& s) const;

	v8::Handle< v8::Value > createObject(ITypedObject* object, bool weakReference) const;

	v8::Handle< v8::Value > toValue(const Any& value) const;

	Any fromValue(v8::Handle< v8::Value > value) const;
};

	}
}

#endif	// traktor_script_ScriptContextJs_H
