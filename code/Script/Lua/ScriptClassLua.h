/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_ScriptClassLua_H
#define traktor_script_ScriptClassLua_H

#include "Core/Class/IRuntimeClass.h"
#include "Core/Containers/AlignedVector.h"

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

	virtual const TypeInfo& getExportType() const T_OVERRIDE T_FINAL;

	virtual const IRuntimeDispatch* getConstructorDispatch() const T_OVERRIDE T_FINAL;

	virtual uint32_t getConstantCount() const T_OVERRIDE T_FINAL;

	virtual std::string getConstantName(uint32_t constId) const T_OVERRIDE T_FINAL;

	virtual Any getConstantValue(uint32_t constId) const T_OVERRIDE T_FINAL;

	virtual uint32_t getMethodCount() const T_OVERRIDE T_FINAL;

	virtual std::string getMethodName(uint32_t methodId) const T_OVERRIDE T_FINAL;

	virtual const IRuntimeDispatch* getMethodDispatch(uint32_t methodId) const T_OVERRIDE T_FINAL;

	virtual uint32_t getStaticMethodCount() const T_OVERRIDE T_FINAL;

	virtual std::string getStaticMethodName(uint32_t methodId) const T_OVERRIDE T_FINAL;

	virtual const IRuntimeDispatch* getStaticMethodDispatch(uint32_t methodId) const T_OVERRIDE T_FINAL;

	virtual uint32_t getPropertiesCount() const T_OVERRIDE T_FINAL;

	virtual std::string getPropertyName(uint32_t propertyId) const T_OVERRIDE T_FINAL;

	virtual const IRuntimeDispatch* getPropertyGetDispatch(uint32_t propertyId) const T_OVERRIDE T_FINAL;

	virtual const IRuntimeDispatch* getPropertySetDispatch(uint32_t propertyId) const T_OVERRIDE T_FINAL;

	virtual const IRuntimeDispatch* getUnknownDispatch() const T_OVERRIDE T_FINAL;

	virtual const IRuntimeDispatch* getOperatorDispatch(OperatorType op) const T_OVERRIDE T_FINAL;

private:
	struct Method
	{
		std::string name;
		Ref< const IRuntimeDispatch > dispatch;
	};

	ScriptManagerLua* m_scriptManager;
	ScriptContextLua* m_scriptContext;
	lua_State*& m_luaState;
	Ref< const IRuntimeDispatch > m_constructor;
	AlignedVector< Method > m_methods;

	ScriptClassLua(ScriptManagerLua* scriptManager, ScriptContextLua* scriptContext, lua_State*& luaState);
};

	}
}

#endif	// traktor_script_ScriptClassLua_H
