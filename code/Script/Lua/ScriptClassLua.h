/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_ScriptClassLua_H
#define traktor_script_ScriptClassLua_H

#include "Core/Class/IRuntimeClass.h"
#include "Core/Containers/SmallMap.h"

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
	static Ref< ScriptClassLua > createFromStack(ScriptManagerLua* scriptManager, ScriptContextLua* scriptContext, lua_State*& luaState);

	virtual ~ScriptClassLua();

	virtual const TypeInfo& getExportType() const T_OVERRIDE T_FINAL;

	virtual bool haveConstructor() const T_OVERRIDE T_FINAL;

	virtual bool haveUnknown() const T_OVERRIDE T_FINAL;

	virtual Ref< ITypedObject > construct(ITypedObject* self, uint32_t argc, const Any* argv, const prototype_t& proto) const T_OVERRIDE T_FINAL;

	virtual uint32_t getConstantCount() const T_OVERRIDE T_FINAL;

	virtual std::string getConstantName(uint32_t constId) const T_OVERRIDE T_FINAL;

	virtual Any getConstantValue(uint32_t constId) const T_OVERRIDE T_FINAL;

	virtual uint32_t getMethodCount() const T_OVERRIDE T_FINAL;

	virtual std::string getMethodName(uint32_t methodId) const T_OVERRIDE T_FINAL;

	virtual std::wstring getMethodSignature(uint32_t methodId) const T_OVERRIDE T_FINAL;

	virtual Any invoke(ITypedObject* object, uint32_t methodId, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL;

	virtual uint32_t getStaticMethodCount() const T_OVERRIDE T_FINAL;

	virtual std::string getStaticMethodName(uint32_t methodId) const T_OVERRIDE T_FINAL;

	virtual std::wstring getStaticMethodSignature(uint32_t methodId) const T_OVERRIDE T_FINAL;

	virtual Any invokeStatic(uint32_t methodId, uint32_t argc, const Any* argv) const T_OVERRIDE T_FINAL;

	virtual uint32_t getPropertiesCount() const T_OVERRIDE T_FINAL;

	virtual std::string getPropertyName(uint32_t propertyId) const T_OVERRIDE T_FINAL;

	virtual std::wstring getPropertySignature(uint32_t propertyId) const T_OVERRIDE T_FINAL;

	virtual Any invokePropertyGet(ITypedObject* self, uint32_t propertyId) const T_OVERRIDE T_FINAL;

	virtual void invokePropertySet(ITypedObject* self, uint32_t propertyId, const Any& value) const T_OVERRIDE T_FINAL;

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
	int32_t m_classRef;
	AlignedVector< Method > m_methods;
	SmallMap< std::string, uint32_t > m_methodLookup;

	ScriptClassLua(ScriptManagerLua* scriptManager, ScriptContextLua* scriptContext, lua_State*& luaState);
};

	}
}

#endif	// traktor_script_ScriptClassLua_H
