#include <cstring>
#include <map>
#include "Core/Guid.h"
#include "Core/Class/Boxes.h"
#include "Core/Math/Format.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Timer/Timer.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmEnum.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfmPrimitive.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Script/Value.h"
#include "Script/ValueObject.h"
#include "Script/Variable.h"
#include "Script/StackFrame.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptDebuggerLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

const int32_t c_tableKey_instance = -2;

//Ref< Local > describeNative(const std::wstring& name, const ISerializable* s, std::map< const void*, Ref< Local > >& cyclic)
//{
//	Ref< Reflection > reflection = Reflection::create(s);
//	if (!reflection)
//		return;
//
//	Ref< LocalComposite > composite = new LocalComposite(name, L"(" + std::wstring(type_name(s)) + L")");
//
//	for (uint32_t i = 0; i < reflection->getMemberCount(); ++i)
//	{
//		const ReflectionMember* member = reflection->getMember(i);
//		T_ASSERT (member);
//
//		if (const RfmObject* memberObject = dynamic_type_cast< const RfmObject* >(member))
//		{
//			std::map< const void*, Ref< Local > >::const_iterator it = cyclic.find(memberObject->get());
//			if (it == cyclic.end())
//			{
//				Ref< Local > child = describeNative(member->getName(), memberObject->get(), cyclic);
//				if (child)
//					composite->addChild(child);
//			}
//		}
//		else if (const RfmPrimitiveBoolean* memberBoolean = dynamic_type_cast< const RfmPrimitiveBoolean* >(member))
//			composite->addChild(new LocalSimple(member->getName(), toString(memberBoolean->get())));
//		else if (const RfmPrimitiveInt8* memberInt8 = dynamic_type_cast< const RfmPrimitiveInt8* >(member))
//			composite->addChild(new LocalSimple(member->getName(), toString< int32_t >(memberInt8->get())));
//		else if (const RfmPrimitiveUInt8* memberUInt8 = dynamic_type_cast< const RfmPrimitiveUInt8* >(member))
//			composite->addChild(new LocalSimple(member->getName(), toString< uint32_t >(memberUInt8->get())));
//		else if (const RfmPrimitiveInt16* memberInt16 = dynamic_type_cast< const RfmPrimitiveInt16* >(member))
//			composite->addChild(new LocalSimple(member->getName(), toString< int32_t >(memberInt16->get())));
//		else if (const RfmPrimitiveUInt16* memberUInt16 = dynamic_type_cast< const RfmPrimitiveUInt16* >(member))
//			composite->addChild(new LocalSimple(member->getName(), toString< uint32_t >(memberUInt16->get())));
//		else if (const RfmPrimitiveInt32* memberInt32 = dynamic_type_cast< const RfmPrimitiveInt32* >(member))
//			composite->addChild(new LocalSimple(member->getName(), toString< int32_t >(memberInt32->get())));
//		else if (const RfmPrimitiveUInt32* memberUInt32 = dynamic_type_cast< const RfmPrimitiveUInt32* >(member))
//			composite->addChild(new LocalSimple(member->getName(), toString< uint32_t >(memberUInt32->get())));
//		else if (const RfmPrimitiveInt64* memberInt64 = dynamic_type_cast< const RfmPrimitiveInt64* >(member))
//			composite->addChild(new LocalSimple(member->getName(), toString(memberInt64->get())));
//		else if (const RfmPrimitiveUInt64* memberUInt64 = dynamic_type_cast< const RfmPrimitiveUInt64* >(member))
//			composite->addChild(new LocalSimple(member->getName(), toString(memberUInt64->get())));
//		else if (const RfmPrimitiveFloat* memberFloat = dynamic_type_cast< const RfmPrimitiveFloat* >(member))
//			composite->addChild(new LocalSimple(member->getName(), toString(memberFloat->get())));
//		else if (const RfmPrimitiveDouble* memberDouble = dynamic_type_cast< const RfmPrimitiveDouble* >(member))
//			composite->addChild(new LocalSimple(member->getName(), toString(memberDouble->get())));
//		else if (const RfmPrimitiveString* memberString = dynamic_type_cast< const RfmPrimitiveString* >(member))
//			composite->addChild(new LocalSimple(member->getName(), mbstows(memberString->get())));
//		else if (const RfmPrimitiveWideString* memberWideString = dynamic_type_cast< const RfmPrimitiveWideString* >(member))
//			composite->addChild(new LocalSimple(member->getName(), memberWideString->get()));
//		else if (const RfmPrimitiveGuid* memberGuid = dynamic_type_cast< const RfmPrimitiveGuid* >(member))
//			composite->addChild(new LocalSimple(member->getName(), memberGuid->get().format()));
//		else if (const RfmPrimitivePath* memberPath = dynamic_type_cast< const RfmPrimitivePath* >(member))
//			composite->addChild(new LocalSimple(member->getName(), memberPath->get().getPathName()));
//		else if (const RfmPrimitiveColor4ub* memberColor4ub = dynamic_type_cast< const RfmPrimitiveColor4ub* >(member))
//			composite->addChild(new LocalSimple(member->getName(), L"(color)"));
//		else if (const RfmPrimitiveColor4f* memberColor4f = dynamic_type_cast< const RfmPrimitiveColor4f* >(member))
//			composite->addChild(new LocalSimple(member->getName(), L"(color)"));
//		else if (const RfmPrimitiveScalar* memberScalar = dynamic_type_cast< const RfmPrimitiveScalar* >(member))
//			composite->addChild(new LocalSimple(member->getName(), toString< float >(memberScalar->get())));
//		else if (const RfmPrimitiveVector2* memberVector2 = dynamic_type_cast< const RfmPrimitiveVector2* >(member))
//			composite->addChild(new LocalSimple(member->getName(), toString(memberVector2->get())));
//		else if (const RfmPrimitiveVector4* memberVector4 = dynamic_type_cast< const RfmPrimitiveVector4* >(member))
//			composite->addChild(new LocalSimple(member->getName(), toString(memberVector4->get())));
//		else if (const RfmPrimitiveMatrix33* memberMatrix33 = dynamic_type_cast< const RfmPrimitiveMatrix33* >(member))
//			composite->addChild(new LocalSimple(member->getName(), L"(matrix33)"));
//		else if (const RfmPrimitiveMatrix44* memberMatrix44 = dynamic_type_cast< const RfmPrimitiveMatrix44* >(member))
//			composite->addChild(new LocalSimple(member->getName(), L"(matrix44)"));
//		else if (const RfmPrimitiveQuaternion* memberQuaternion = dynamic_type_cast< const RfmPrimitiveQuaternion* >(member))
//			composite->addChild(new LocalSimple(member->getName(), toString(memberQuaternion->get())));
//		else if (const RfmEnum* memberEnum = dynamic_type_cast< const RfmEnum* >(member))
//			composite->addChild(new LocalSimple(member->getName(), memberEnum->get()));
//		else
//			composite->addChild(new LocalSimple(member->getName(), L"(...)"));
//	}
//}
//
//Ref< Variable > describeLocal(const std::wstring& name, lua_State* L, int32_t index, int32_t depth)
//{
//	T_ANONYMOUS_VAR(UnwindStack)(L);
//	Ref< Variable > variable = new Variable(name, L"", 0);
//
//	if (lua_isnumber(L, index))
//	{
//		variable->setTypeName(L"(number)");
//		variable->setValue(new Value(toString(lua_tonumber(L, index))));
//	}
//	else if (lua_isboolean(L, index))
//	{
//		variable->setTypeName(L"(boolean)");
//		variable->setValue(new Value(lua_toboolean(L, index) != 0 ? L"true" : L"false"));
//	}
//	else if (lua_isstring(L, index))
//	{
//		variable->setTypeName(L"(string)");
//		variable->setValue(new Value(mbstows(lua_tostring(L, index))));
//	}
//	else if (lua_isfunction(L, index))
//	{
//		variable->setTypeName(L"(function)");
//	}
//	else if (lua_istable(L, index))
//	{
//		Ref< ValueComposite > compositeValue = new ValueComposite();
//
//		// Native object.
//		lua_rawgeti(L, index, c_tableKey_instance);
//		if (lua_islightuserdata(L, -1))
//		{
//			ITypedObject* object = reinterpret_cast< ITypedObject* >(lua_touserdata(L, -1));
//			lua_pop(L, 1);
//
//			if (object)
//			{
//				variable->setTypeName(type_name(object));
//				/*if (const ISerializable* s = dynamic_type_cast< const ISerializable* >(object))
//				{
//					Ref< Reflection > reflection = Reflection::create(s);
//					if (reflection)
//						describeCompound(composite, reflection, cyclic);
//				}
//				else*/ if (const Boxed* b = dynamic_type_cast< const Boxed* >(object))
//					compositeValue->add(new Variable(L"(value)", L"", new Value(b->toString())));
//			}
//		}
//		else
//			lua_pop(L, 1);
//
//		//if (lua_getmetatable(L, index) != 0)
//		//{
//		//	Ref< Local > metaLocal = describeLocal(L"(meta)", L, -1, cyclic);
//		//	if (metaLocal)
//		//		composite->addChild(metaLocal);
//		//	lua_pop(L, 1);
//		//}
//
//		lua_pushnil(L);
//		while (lua_next(L, index - 1))
//		{
//			std::wstring name;
//
//			if (lua_isnumber(L, -2))
//				name = toString(lua_tonumber(L, -2));
//			else if (lua_isboolean(L, -2))
//				name = lua_toboolean(L, -2) != 0 ? L"true" : L"false";
//			else if (lua_isstring(L, -2))
//				name = mbstows(lua_tostring(L, -2));
//			else
//			{
//				lua_pop(L, 1);
//				continue;
//			}
//
//			// Hide internal keys from debugger.
//			if (name == L"-1" || name == L"-2")
//			{
//				lua_pop(L, 1);
//				continue;
//			}
//
//			if (depth < 2)
//			{
//				Ref< Variable > childValue = describeLocal(name, L, -1, depth + 1);
//				if (childValue)
//					compositeValue->add(childValue);
//			}
//			else
//				compositeValue->add(new Variable(name, L"", new Value(L"(...)")));
//
//			lua_pop(L, 1);
//		}
//
//		variable->setValue(compositeValue);
//	}
//
//	return variable;
//}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptDebuggerLua", ScriptDebuggerLua, IScriptDebugger)

ScriptDebuggerLua::ScriptDebuggerLua(ScriptManagerLua* scriptManager, lua_State* luaState)
:	m_scriptManager(scriptManager)
,	m_luaState(luaState)
,	m_state(StRunning)
{
}

ScriptDebuggerLua::~ScriptDebuggerLua()
{
}

bool ScriptDebuggerLua::setBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_breakpoints[lineNumber].insert(scriptId);
	return true;
}

bool ScriptDebuggerLua::removeBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_breakpoints[lineNumber].erase(scriptId);
	return true;
}

void ScriptDebuggerLua::addListener(IListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_listeners.insert(listener);
}

void ScriptDebuggerLua::removeListener(IListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_listeners.erase(listener);

	// If all listeners have been removed then automatically continue running;
	// don't want the application to be kept locking up running thread.
	if (m_listeners.empty())
		m_state = StRunning;
}

bool ScriptDebuggerLua::captureStackFrame(uint32_t depth, Ref< StackFrame >& outStackFrame)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	ScriptContextLua* currentContext = m_scriptManager->m_lockContext;
	if (!currentContext)
		return false;

	lua_Debug ar = { 0 };
	if (!lua_getstack(currentContext->m_luaState, depth, &ar))
		return false;

	lua_getinfo(currentContext->m_luaState, "Snlu", &ar);

	outStackFrame = new StackFrame();
	outStackFrame->setScriptId(Guid(mbstows(ar.source)));
	outStackFrame->setFunctionName(ar.name ? mbstows(ar.name) : L"(anonymous)");
	outStackFrame->setLine(max(ar.currentline - 1, 0));

	return true;
}

bool ScriptDebuggerLua::captureLocals(uint32_t depth, RefArray< Variable >& outLocals)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	ScriptContextLua* currentContext = m_scriptManager->m_lockContext;
	if (!currentContext)
		return false;

	lua_State* L = currentContext->m_luaState;
	T_ANONYMOUS_VAR(UnwindStack)(L);

	lua_Debug ar = { 0 };
	if (!lua_getstack(L, depth, &ar))
		return false;

	const char* localName;
	for (int n = 1; (localName = lua_getlocal(L, &ar, n)) != 0; ++n)
	{
		if (*localName != '(')
		{
			Ref< Variable > variable = new Variable(mbstows(localName), L"", 0);

			if (lua_isnumber(L, -1))
			{
				variable->setTypeName(L"(number)");
				variable->setValue(new Value(toString(lua_tonumber(L, -1))));
				lua_pop(L, 1);
			}
			else if (lua_isboolean(L, -1))
			{
				variable->setTypeName(L"(boolean)");
				variable->setValue(new Value(lua_toboolean(L, -1) != 0 ? L"true" : L"false"));
				lua_pop(L, 1);
			}
			else if (lua_isstring(L, -1))
			{
				variable->setTypeName(L"(string)");
				variable->setValue(new Value(mbstows(lua_tostring(L, -1))));
				lua_pop(L, 1);
			}
			else if (lua_isfunction(L, -1))
			{
				variable->setTypeName(L"(function)");
				lua_pop(L, 1);
			}
			else if (lua_istable(L, -1))
			{
				lua_rawgeti(L, -1, c_tableKey_instance);
				if (lua_islightuserdata(L, -1))
				{
					ITypedObject* object = reinterpret_cast< ITypedObject* >(lua_touserdata(L, -1));
					lua_pop(L, 1);

					if (object)
						variable->setTypeName(type_name(object));
				}
				else
					lua_pop(L, 1);

				uint32_t objectRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
				variable->setValue(new ValueObject(objectRef));
			}
			else
				lua_pop(L, 1);

			outLocals.push_back(variable);
		}
		else
			lua_pop(currentContext->m_luaState, 1);
	}

	return true;
}

bool ScriptDebuggerLua::captureObject(uint32_t object, RefArray< Variable >& outMembers)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	ScriptContextLua* currentContext = m_scriptManager->m_lockContext;
	if (!currentContext)
		return false;

	lua_State* L = currentContext->m_luaState;
	T_ANONYMOUS_VAR(UnwindStack)(L);

	lua_rawgeti(L, LUA_REGISTRYINDEX, object);
	T_ASSERT (lua_istable(L, -1));

	lua_pushnil(L);
	while (lua_next(L, -2))
	{
		std::wstring name;

		if (lua_isnumber(L, -2))
			name = toString(lua_tonumber(L, -2));
		else if (lua_isboolean(L, -2))
			name = lua_toboolean(L, -2) != 0 ? L"true" : L"false";
		else if (lua_isstring(L, -2))
			name = mbstows(lua_tostring(L, -2));
		else
		{
			lua_pop(L, 1);
			continue;
		}

		if (name == L"-1" || name == L"-2")
		{
			lua_pop(L, 1);
			continue;
		}

		Ref< Variable > variable = new Variable(name, L"", 0);

		if (lua_isnumber(L, -1))
		{
			variable->setTypeName(L"(number)");
			variable->setValue(new Value(toString(lua_tonumber(L, -1))));
			lua_pop(L, 1);
		}
		else if (lua_isboolean(L, -1))
		{
			variable->setTypeName(L"(boolean)");
			variable->setValue(new Value(lua_toboolean(L, -1) != 0 ? L"true" : L"false"));
			lua_pop(L, 1);
		}
		else if (lua_isstring(L, -1))
		{
			variable->setTypeName(L"(string)");
			variable->setValue(new Value(mbstows(lua_tostring(L, -1))));
			lua_pop(L, 1);
		}
		else if (lua_isfunction(L, -1))
		{
			variable->setTypeName(L"(function)");
			lua_pop(L, 1);
		}
		else if (lua_istable(L, -1))
		{
			lua_rawgeti(L, -1, c_tableKey_instance);
			if (lua_islightuserdata(L, -1))
			{
				ITypedObject* object = reinterpret_cast< ITypedObject* >(lua_touserdata(L, -1));
				lua_pop(L, 1);

				if (object)
					variable->setTypeName(type_name(object));
			}
			else
				lua_pop(L, 1);

			uint32_t objectRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
			variable->setValue(new ValueObject(objectRef));
		}
		else
			lua_pop(L, 1);

		outMembers.push_back(variable);
	}

	return true;
}

bool ScriptDebuggerLua::isRunning() const
{
	return m_state == StRunning;
}

bool ScriptDebuggerLua::actionBreak()
{
	m_state = StBreak;
	return true;
}

bool ScriptDebuggerLua::actionContinue()
{
	m_state = StRunning;
	return true;
}

bool ScriptDebuggerLua::actionStepInto()
{
	m_state = StStepInto;
	return false;
}

bool ScriptDebuggerLua::actionStepOver()
{
	m_state = StStepOver;
	return false;
}

void ScriptDebuggerLua::analyzeState(lua_State* L, lua_Debug* ar)
{
	ScriptContextLua* currentContext = m_scriptManager->m_lockContext;
	if (!currentContext)
		return;

	if (m_state == StRunning)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		int32_t currentLine = ar->currentline - 1;
		Guid currentId;

		// Any breakpoint defined with current line number?
		SmallMap< int32_t, SmallSet< Guid > >::const_iterator i = m_breakpoints.find(currentLine);
		if (i != m_breakpoints.end())
		{
			// Get executing script's identifier.
			lua_getinfo(L, "S", ar);
			if (currentId.create(mbstows(ar->source)))
			{
				// If identifier also match then we halt and trigger "breakpoint reached".
				if (i->second.find(currentId) != i->second.end())
				{
					m_state = StHalted;
					m_lastId = currentId;

					for (SmallSet< IListener* >::const_iterator j = m_listeners.begin(); j != m_listeners.end(); ++j)
						(*j)->debugeeStateChange(this);
				}

			}
		}
	}
	else if (m_state == StBreak)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		Guid currentId;

		// Get executing script's identifier.
		lua_getinfo(L, "S", ar);
		if (currentId.create(mbstows(ar->source)))
		{
			m_state = StHalted;
			m_lastId = currentId;
			for (SmallSet< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
				(*i)->debugeeStateChange(this);
		}
	}
	else if (m_state == StStepInto)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		Guid currentId;

		// Get executing script's identifier.
		lua_getinfo(L, "S", ar);
		if (currentId.create(mbstows(ar->source)))
		{
			m_state = StHalted;
			m_lastId = currentId;
			for (SmallSet< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
				(*i)->debugeeStateChange(this);
		}
	}
	else if (m_state == StStepOver)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		Guid currentId;

		// Get executing script's identifier.
		lua_getinfo(L, "S", ar);
		if (currentId.create(mbstows(ar->source)))
		{
			if (currentId == m_lastId)
			{
				m_state = StHalted;
				m_lastId = currentId;
				for (SmallSet< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
					(*i)->debugeeStateChange(this);
			}
		}
	}

	if (m_state == StHalted)
	{
		// Wait until state is no longer halted.
		Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
		do
		{
			currentThread->sleep(100);
		}
		while (m_state == StHalted && !currentThread->stopped());

		for (SmallSet< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
			(*i)->debugeeStateChange(this);
	}
}

void ScriptDebuggerLua::hookCallback(lua_State* L, lua_Debug* ar)
{
	if (ar->event == LUA_HOOKLINE)
		analyzeState(L, ar);
}

	}
}
