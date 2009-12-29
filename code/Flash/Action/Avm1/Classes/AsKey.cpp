#include <algorithm>
#include <cstring>
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/Avm1/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsKey.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsKey", AsKey, ActionClass)

Ref< AsKey > AsKey::createInstance()
{
	Ref< AsKey > instance = new AsKey();
	instance->createPrototype();
	instance->setReadOnly();
	return instance;
}

AsKey::AsKey()
:	ActionClass(L"Key")
,	m_lastKeyCode(0)
{
	std::memset(m_keyState, 0, sizeof(m_keyState));
}

void AsKey::eventKeyDown(IActionVM* actionVM, ActionContext* context, int keyCode)
{
	m_keyState[keyCode] = true;
	m_lastKeyCode = keyCode;

	// Create a snapshot of active listeners the moment this event is raised,
	// this because listeners can be either added or removed by listeners.
	RefArray< ActionObject > listeners = m_listeners;
	for (RefArray< ActionObject >::iterator i = listeners.begin(); i != listeners.end(); ++i)
	{
		ActionValue member;
		(*i)->getMember(L"onKeyDown", member);
		if (member.isUndefined())
			continue;

		Ref< ActionFunction > eventFunction = checked_type_cast< ActionFunction* >(member.getObject());
		if (eventFunction)
		{
			ActionFrame callerFrame(context, 0, 0, 0, 4, 0, 0);
			eventFunction->call(actionVM, &callerFrame, (*i));
		}
	}
}

void AsKey::eventKeyUp(IActionVM* actionVM, ActionContext* context, int keyCode)
{
	m_keyState[keyCode] = false;

	// Create a snapshot of active listeners the moment this event is raised,
	// this because listeners can be either added or removed by listeners.
	RefArray< ActionObject > listeners = m_listeners;
	for (RefArray< ActionObject >::iterator i = listeners.begin(); i != listeners.end(); ++i)
	{
		ActionValue member;
		(*i)->getMember(L"onKeyUp", member);
		if (member.isUndefined())
			continue;

		Ref< ActionFunction > eventFunction = checked_type_cast< ActionFunction* >(member.getObject());
		if (eventFunction)
		{
			ActionFrame callerFrame(context, 0, 0, 0, 4, 0, 0);
			eventFunction->call(actionVM, &callerFrame, (*i));
		}
	}
}

void AsKey::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"BACKSPACE", ActionValue(8.0));
	prototype->setMember(L"CAPSLOCK", ActionValue(20.0));
	prototype->setMember(L"CONTROL", ActionValue(17.0));
	prototype->setMember(L"DELETEKEY", ActionValue(46.0));
	prototype->setMember(L"DOWN", ActionValue(40.0));
	prototype->setMember(L"END", ActionValue(35.0));
	prototype->setMember(L"ENTER", ActionValue(13.0));
	prototype->setMember(L"ESCAPE", ActionValue(27.0));
	prototype->setMember(L"HOME", ActionValue(36.0));
	prototype->setMember(L"INSERT", ActionValue(45.0));
	prototype->setMember(L"LEFT", ActionValue(37.0));
	prototype->setMember(L"PGDN", ActionValue(34.0));
	prototype->setMember(L"PGUP", ActionValue(33.0));
	prototype->setMember(L"RIGHT", ActionValue(39.0));
	prototype->setMember(L"SHIFT", ActionValue(16.0));
	prototype->setMember(L"SPACE", ActionValue(32.0));
	prototype->setMember(L"TAB", ActionValue(9.0));
	prototype->setMember(L"UP", ActionValue(38.0));
	prototype->setMember(L"addListener", createNativeFunctionValue(this, &AsKey::Key_addListener));
	prototype->setMember(L"getAscii", createNativeFunctionValue(this, &AsKey::Key_getAscii));
	prototype->setMember(L"getCode", createNativeFunctionValue(this, &AsKey::Key_getCode));
	prototype->setMember(L"isAccessible", createNativeFunctionValue(this, &AsKey::Key_isAccessible));
	prototype->setMember(L"isDown", createNativeFunctionValue(this, &AsKey::Key_isDown));
	prototype->setMember(L"isToggled", createNativeFunctionValue(this, &AsKey::Key_isToggled));
	prototype->setMember(L"removeListener", createNativeFunctionValue(this, &AsKey::Key_removeListener));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsKey::construct(ActionContext* context, const args_t& args)
{
	return ActionValue();
}

void AsKey::Key_addListener(CallArgs& ca)
{
	m_listeners.push_back(ca.args[0].getObjectSafe());
}

void AsKey::Key_getAscii(CallArgs& ca)
{
	ca.ret = ActionValue(double(m_lastKeyCode));
}

void AsKey::Key_getCode(CallArgs& ca)
{
	ca.ret = ActionValue(double(m_lastKeyCode));
}

void AsKey::Key_isAccessible(CallArgs& ca)
{
	ca.ret = ActionValue(false);
}

void AsKey::Key_isDown(CallArgs& ca)
{
	int keyCode = int(ca.args[0].getNumber());
	ca.ret = ActionValue((keyCode >= 0 && keyCode < 256) ? m_keyState[keyCode] : false);
}

void AsKey::Key_isToggled(CallArgs& ca)
{
	ca.ret = ActionValue(false);
}

void AsKey::Key_removeListener(CallArgs& ca)
{
	RefArray< ActionObject >::iterator i = std::find(m_listeners.begin(), m_listeners.end(), ca.args[0].getObjectSafe());
	if (i != m_listeners.end())
		m_listeners.erase(i);
}

	}
}
