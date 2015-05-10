#ifndef traktor_script_ScriptClassLua_H
#define traktor_script_ScriptClassLua_H

#include "Core/Class/IRuntimeClass.h"

struct lua_State;

namespace traktor
{
	namespace script
	{

class ScriptContextLua;

class ScriptClassLua : public IRuntimeClass
{
	T_RTTI_CLASS;

public:
	ScriptClassLua(ScriptContextLua* context, lua_State*& luaState, const std::string& className);

	void addMethod(const std::string& name, int32_t ref);

	virtual const TypeInfo& getExportType() const;

	virtual bool haveConstructor() const;

	virtual bool haveUnknown() const;

	virtual Ref< ITypedObject > construct(uint32_t argc, const Any* argv) const;

	virtual uint32_t getMethodCount() const;

	virtual std::string getMethodName(uint32_t methodId) const;

	virtual Any invoke(ITypedObject* object, uint32_t methodId, uint32_t argc, const Any* argv) const;

	virtual uint32_t getStaticMethodCount() const;

	virtual std::string getStaticMethodName(uint32_t methodId) const;

	virtual Any invokeStatic(uint32_t methodId, uint32_t argc, const Any* argv) const;

	virtual Any invokeUnknown(ITypedObject* object, const std::string& methodName, uint32_t argc, const Any* argv) const;

	virtual Any invokeOperator(ITypedObject* object, uint8_t operation, const Any& arg) const;

private:
	struct Method
	{
		std::string name;
		int32_t ref;
	};

	ScriptContextLua* m_context;
	lua_State*& m_luaState;
	std::string m_className;
	std::vector< Method > m_methods;
};

	}
}

#endif	// traktor_script_ScriptClassLua_H
