/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Class/IRuntimeClass.h"
#include "Core/Containers/AlignedVector.h"

struct lua_State;

namespace traktor::script
{

class ScriptContextLua;
class ScriptManagerLua;

/*! Runtime representation of a script-side LUA class.
 * \ingroup Script
 */
class ScriptClassLua : public IRuntimeClass
{
	T_RTTI_CLASS;

public:
	static Ref< ScriptClassLua > createFromStack(ScriptManagerLua* scriptManager, ScriptContextLua* scriptContext, lua_State*& luaState);

	virtual const TypeInfo& getExportType() const override final;

	virtual const IRuntimeDispatch* getConstructorDispatch() const override final;

	virtual uint32_t getConstantCount() const override final;

	virtual std::string getConstantName(uint32_t constId) const override final;

	virtual Any getConstantValue(uint32_t constId) const override final;

	virtual uint32_t getMethodCount() const override final;

	virtual std::string getMethodName(uint32_t methodId) const override final;

	virtual const IRuntimeDispatch* getMethodDispatch(uint32_t methodId) const override final;

	virtual uint32_t getStaticMethodCount() const override final;

	virtual std::string getStaticMethodName(uint32_t methodId) const override final;

	virtual const IRuntimeDispatch* getStaticMethodDispatch(uint32_t methodId) const override final;

	virtual uint32_t getPropertiesCount() const override final;

	virtual std::string getPropertyName(uint32_t propertyId) const override final;

	virtual const IRuntimeDispatch* getPropertyGetDispatch(uint32_t propertyId) const override final;

	virtual const IRuntimeDispatch* getPropertySetDispatch(uint32_t propertyId) const override final;

	virtual const IRuntimeDispatch* getOperatorDispatch(Operator op) const override final;

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
