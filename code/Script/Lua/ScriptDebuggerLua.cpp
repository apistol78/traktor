#include <cstring>
#include "Core/Guid.h"
#include "Core/Class/Boxes.h"
#include "Core/Math/Format.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmEnum.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfmPrimitive.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Script/CallStack.h"
#include "Script/LocalComposite.h"
#include "Script/LocalSimple.h"
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

std::wstring describeValue(lua_State* L, int32_t index)
{
	if (lua_isnumber(L, index))
		return toString(lua_tonumber(L, index));
	else if (lua_isboolean(L, index))
		return lua_toboolean(L, index) != 0 ? L"true" : L"false";
	else if (lua_isstring(L, index))
		return mbstows(lua_tostring(L, index));
	else if (lua_isuserdata(L, index))
	{
		Object* object = *reinterpret_cast< Object** >(lua_touserdata(L, index));
		if (object)
			return std::wstring(L"(") + type_name(object) + std::wstring(L")");
		else
			return L"(null)";
	}
	else
		return L"";
}

Ref< Local > describeCompound(const std::wstring& name, const RfmCompound* compound)
{
	RefArray< Local > memberValues;
	for (uint32_t i = 0; i < compound->getMemberCount(); ++i)
	{
		const ReflectionMember* member = compound->getMember(i);
		T_ASSERT (member);

		if (const RfmObject* memberObject = dynamic_type_cast< const RfmObject* >(member))
		{
			Ref< Reflection > reflection = Reflection::create(memberObject->get());
			if (reflection)
				memberValues.push_back(describeCompound(member->getName(), reflection));
			else
				memberValues.push_back(new LocalSimple(member->getName(), L"(null)"));
		}
		else if (const RfmCompound* memberCompound = dynamic_type_cast< const RfmCompound* >(member))
			memberValues.push_back(describeCompound(member->getName(), memberCompound));
		else if (const RfmPrimitiveBoolean* memberBoolean = dynamic_type_cast< const RfmPrimitiveBoolean* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), toString(memberBoolean->get())));
		else if (const RfmPrimitiveInt8* memberInt8 = dynamic_type_cast< const RfmPrimitiveInt8* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), toString< int32_t >(memberInt8->get())));
		else if (const RfmPrimitiveUInt8* memberUInt8 = dynamic_type_cast< const RfmPrimitiveUInt8* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), toString< uint32_t >(memberUInt8->get())));
		else if (const RfmPrimitiveInt16* memberInt16 = dynamic_type_cast< const RfmPrimitiveInt16* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), toString< int32_t >(memberInt16->get())));
		else if (const RfmPrimitiveUInt16* memberUInt16 = dynamic_type_cast< const RfmPrimitiveUInt16* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), toString< uint32_t >(memberUInt16->get())));
		else if (const RfmPrimitiveInt32* memberInt32 = dynamic_type_cast< const RfmPrimitiveInt32* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), toString< int32_t >(memberInt32->get())));
		else if (const RfmPrimitiveUInt32* memberUInt32 = dynamic_type_cast< const RfmPrimitiveUInt32* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), toString< uint32_t >(memberUInt32->get())));
		else if (const RfmPrimitiveInt64* memberInt64 = dynamic_type_cast< const RfmPrimitiveInt64* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), toString(memberInt64->get())));
		else if (const RfmPrimitiveUInt64* memberUInt64 = dynamic_type_cast< const RfmPrimitiveUInt64* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), toString(memberUInt64->get())));
		else if (const RfmPrimitiveFloat* memberFloat = dynamic_type_cast< const RfmPrimitiveFloat* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), toString(memberFloat->get())));
		else if (const RfmPrimitiveDouble* memberDouble = dynamic_type_cast< const RfmPrimitiveDouble* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), toString(memberDouble->get())));
		else if (const RfmPrimitiveString* memberString = dynamic_type_cast< const RfmPrimitiveString* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), mbstows(memberString->get())));
		else if (const RfmPrimitiveWideString* memberWideString = dynamic_type_cast< const RfmPrimitiveWideString* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), memberWideString->get()));
		else if (const RfmPrimitiveGuid* memberGuid = dynamic_type_cast< const RfmPrimitiveGuid* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), memberGuid->get().format()));
		else if (const RfmPrimitivePath* memberPath = dynamic_type_cast< const RfmPrimitivePath* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), memberPath->get().getPathName()));
		else if (const RfmPrimitiveColor4ub* memberColor4ub = dynamic_type_cast< const RfmPrimitiveColor4ub* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), L"(color)"));
		else if (const RfmPrimitiveColor4f* memberColor4f = dynamic_type_cast< const RfmPrimitiveColor4f* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), L"(color)"));
		else if (const RfmPrimitiveScalar* memberScalar = dynamic_type_cast< const RfmPrimitiveScalar* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), toString< float >(memberScalar->get())));
		else if (const RfmPrimitiveVector2* memberVector2 = dynamic_type_cast< const RfmPrimitiveVector2* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), toString(memberVector2->get())));
		else if (const RfmPrimitiveVector4* memberVector4 = dynamic_type_cast< const RfmPrimitiveVector4* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), toString(memberVector4->get())));
		else if (const RfmPrimitiveMatrix33* memberMatrix33 = dynamic_type_cast< const RfmPrimitiveMatrix33* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), L"(matrix33)"));
		else if (const RfmPrimitiveMatrix44* memberMatrix44 = dynamic_type_cast< const RfmPrimitiveMatrix44* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), L"(matrix44)"));
		else if (const RfmPrimitiveQuaternion* memberQuaternion = dynamic_type_cast< const RfmPrimitiveQuaternion* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), toString(memberQuaternion->get())));
		else if (const RfmEnum* memberEnum = dynamic_type_cast< const RfmEnum* >(member))
			memberValues.push_back(new LocalSimple(member->getName(), memberEnum->get()));
		else
			memberValues.push_back(new LocalSimple(member->getName(), L"(...)"));
	}
	return new LocalComposite(name, memberValues);
}

Ref< Local > describeSerializable(const std::wstring& name, const ISerializable* s)
{
	Ref< Reflection > reflection = Reflection::create(s);
	if (reflection)
		return describeCompound(name, reflection);
	else
		return new LocalSimple(name, L"(...)");
}

Ref< Local > describeLocal(const std::wstring& name, lua_State* L, int32_t index, int depth)
{
	if (lua_istable(L, index))
	{
		RefArray< Local > values;

		if (lua_getmetatable(L, index) != 0)
		{
			Ref< Local > metaLocal = describeLocal(L"(meta)", L, -1, depth + 1);
			if (metaLocal)
				values.push_back(metaLocal);
			lua_pop(L, 1);
		}

		lua_pushnil(L);
		while (lua_next(L, index - 1))
		{
			std::wstring name = describeValue(L, -2);
			if (depth < 4)
			{
				Ref< Local > value = describeLocal(name, L, -1, depth + 1);
				if (value)
					values.push_back(value);
			}
			else
				values.push_back(new LocalSimple(name, L"(...)"));
			lua_pop(L, 1);
		}

		return new LocalComposite(
			name,
			values
		);
	}
	else
	{
		if (lua_isnumber(L, index))
			return new LocalSimple(name, toString(lua_tonumber(L, index)));
		
		if (lua_isboolean(L, index))
			return new LocalSimple(name, lua_toboolean(L, index) != 0 ? L"true" : L"false");

		if (lua_isstring(L, index))
			return new LocalSimple(name, mbstows(lua_tostring(L, index)));

		if (lua_isfunction(L, index))
			return new LocalSimple(name, L"(function)");

		if (lua_isuserdata(L, index) && !lua_islightuserdata(L, index))
		{
			ITypedObject* object = *reinterpret_cast< ITypedObject** >(lua_touserdata(L, index));
			if (object)
			{
				if (const Boxed* box = dynamic_type_cast< const Boxed* >(object))
					return new LocalSimple(name, box->toString());

				if (const ISerializable* s = dynamic_type_cast< const ISerializable* >(object))
					return describeSerializable(name, s);

				return new LocalSimple(name, std::wstring(L"(") + type_name(object) + std::wstring(L")"));
			}
		}

		return new LocalSimple(
			name,
			L""
		);
	}
}

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
		std::map< int32_t, std::set< Guid > >::const_iterator i = m_breakpoints.find(currentLine);
		if (i != m_breakpoints.end())
		{
			// Get executing script's identifier.
			lua_getinfo(L, "S", ar);
			if (currentId.create(mbstows(ar->source)))
			{
				// If identifier also match then we halt and trigger "breakpoint reached".
				if (i->second.find(currentId) != i->second.end())
				{
					CallStack cs;
					captureCallStack(L, cs);

					m_state = StHalted;
					m_lastId = currentId;

					for (std::set< IListener* >::const_iterator j = m_listeners.begin(); j != m_listeners.end(); ++j)
						(*j)->breakpointReached(this, cs);
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
			CallStack cs;
			captureCallStack(L, cs);

			m_state = StHalted;
			m_lastId = currentId;

			for (std::set< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
				(*i)->breakpointReached(this, cs);
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
			CallStack cs;
			captureCallStack(L, cs);

			m_state = StHalted;
			m_lastId = currentId;

			for (std::set< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
				(*i)->breakpointReached(this, cs);
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
				CallStack cs;
				captureCallStack(L, cs);

				m_state = StHalted;

				for (std::set< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
					(*i)->breakpointReached(this, cs);
			}
		}
	}

	if (m_state == StHalted)
	{
		Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
		do
		{
			currentThread->sleep(100);
		}
		while (m_state == StHalted && !currentThread->stopped());
	}
}

void ScriptDebuggerLua::captureCallStack(lua_State* L, CallStack& outCallStack)
{
	ScriptContextLua* currentContext = m_scriptManager->m_lockContext;
	T_ASSERT (currentContext);

	lua_Debug ar;
	std::memset(&ar, 0, sizeof(ar));

	for (int level = 0; lua_getstack(L, level, &ar); ++level)
	{
		lua_getinfo(L, "Snlu", &ar);

		Guid currentId(mbstows(ar.source));
		int32_t currentLine = ar.currentline - 1;

		CallStack::Frame f;
		f.scriptId = currentId;
		f.functionName = ar.name ? mbstows(ar.name) : L"(anonymous)";
		f.line = currentLine;

		const char* localName;
		for (int n = 1; (localName = lua_getlocal(L, &ar, n)) != 0; ++n)
		{
			if (*localName != '(')
			{
				Ref< Local > local = describeLocal(mbstows(localName), L, -1, 0);
				if (local)
					f.locals.push_back(local);
			}
			lua_pop(L, 1);
		}

		outCallStack.pushFrame(f);
		std::memset(&ar, 0, sizeof(ar));
	}
}

void ScriptDebuggerLua::hookCallback(lua_State* L, lua_Debug* ar)
{
	if (ar->event == LUA_HOOKLINE)
		analyzeState(L, ar);
}

	}
}
