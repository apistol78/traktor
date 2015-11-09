#ifndef traktor_script_ScriptClassLua_H
#define traktor_script_ScriptClassLua_H

#include "Core/Class/IRuntimeClass.h"

struct lua_State;

namespace traktor
{
	namespace script
	{

class ScriptContextLua;
class ScriptManagerLua;

class ScriptClassLua : public IRuntimeClass
{
	T_RTTI_CLASS;

public:
	ScriptClassLua(ScriptManagerLua* scriptManager, ScriptContextLua* scriptContext, lua_State*& luaState, const std::string& className);

	virtual ~ScriptClassLua();

	void addMethod(const std::string& name, int32_t ref);

	virtual const TypeInfo& getExportType() const T_OVERRIDE T_FINAL;

	virtual bool haveConstructor() const T_OVERRIDE T_FINAL;

	virtual bool haveUnknown() const T_OVERRIDE T_FINAL;

	virtual Ref< ITypedObject > construct(ITypedObject* self, uint32_t argc, const Any* argv, const prototype_t& proto) const T_OVERRIDE T_FINAL;

	virtual uint32_t getConstantCount() const T_OVERRIDE T_FINAL;

	virtual std::string getConstantName(uint32_t constId) const T_OVERRIDE T_FINAL;

	virtual Any getConstantValue(uint32_t constId) const T_OVERRIDE T_FINAL;

	virtual uint32_t getMethodCount() const T_OVERRIDE T_FINAL;

	virtual std::string getMethodName(uint32_t methodId) const T_OVERRIDE T_FINAL;

	virtual void getMethodSignature(uint32_t methodId, const wchar_t* outSignature[MaxSignatures]) const T_OVERRIDE T_FINAL;

	virtual Any invoke(ITypedObject* object, uint32_t methodId, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL;

	virtual uint32_t getStaticMethodCount() const T_OVERRIDE T_FINAL;

	virtual std::string getStaticMethodName(uint32_t methodId) const T_OVERRIDE T_FINAL;

	virtual void getStaticMethodSignature(uint32_t methodId, const wchar_t* outSignature[MaxSignatures]) const T_OVERRIDE T_FINAL;

	virtual Any invokeStatic(uint32_t methodId, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL;

	virtual Any invokeUnknown(ITypedObject* object, const std::string& methodName, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL;

	virtual Any invokeOperator(ITypedObject* object, uint8_t operation, const Any& arg) const T_OVERRIDE T_FINAL;

private:
	struct Method
	{
		std::string name;
		int32_t ref;
	};

	ScriptManagerLua* m_scriptManager;
	ScriptContextLua* m_scriptContext;
	lua_State*& m_luaState;
	std::string m_className;
	std::vector< Method > m_methods;
};

	}
}

#endif	// traktor_script_ScriptClassLua_H
