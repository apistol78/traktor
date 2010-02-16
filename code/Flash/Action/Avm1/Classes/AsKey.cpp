#include <algorithm>
#include <cstring>
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunctionNative.h"
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

AsKey::~AsKey()
{
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

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"BACKSPACE", ActionValue(avm_number_t(AkBackspace)));
	prototype->setMember(L"CAPSLOCK", ActionValue(avm_number_t(AkCapsLock)));
	prototype->setMember(L"CONTROL", ActionValue(avm_number_t(AkControl)));
	prototype->setMember(L"DELETEKEY", ActionValue(avm_number_t(AkDeleteKey)));
	prototype->setMember(L"DOWN", ActionValue(avm_number_t(AkDown)));
	prototype->setMember(L"END", ActionValue(avm_number_t(AkEnd)));
	prototype->setMember(L"ENTER", ActionValue(avm_number_t(AkEnter)));
	prototype->setMember(L"ESCAPE", ActionValue(avm_number_t(AkEscape)));
	prototype->setMember(L"HOME", ActionValue(avm_number_t(AkHome)));
	prototype->setMember(L"INSERT", ActionValue(avm_number_t(AkInsert)));
	prototype->setMember(L"LEFT", ActionValue(avm_number_t(AkLeft)));
	prototype->setMember(L"PGDN", ActionValue(avm_number_t(AkPgDn)));
	prototype->setMember(L"PGUP", ActionValue(avm_number_t(AkPgUp)));
	prototype->setMember(L"RIGHT", ActionValue(avm_number_t(AkRight)));
	prototype->setMember(L"SHIFT", ActionValue(avm_number_t(AkShift)));
	prototype->setMember(L"SPACE", ActionValue(avm_number_t(AkSpace)));
	prototype->setMember(L"TAB", ActionValue(avm_number_t(AkTab)));
	prototype->setMember(L"UP", ActionValue(avm_number_t(AkUp)));
	prototype->setMember(L"addListener", createNativeFunctionValue(this, &AsKey::Key_addListener));
	prototype->setMember(L"getAscii", createNativeFunctionValue(this, &AsKey::Key_getAscii));
	prototype->setMember(L"getCode", createNativeFunctionValue(this, &AsKey::Key_getCode));
	prototype->setMember(L"isAccessible", createNativeFunctionValue(this, &AsKey::Key_isAccessible));
	prototype->setMember(L"isDown", createNativeFunctionValue(this, &AsKey::Key_isDown));
	prototype->setMember(L"isToggled", createNativeFunctionValue(this, &AsKey::Key_isToggled));
	prototype->setMember(L"removeListener", createNativeFunctionValue(this, &AsKey::Key_removeListener));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
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
	ca.ret = ActionValue(avm_number_t(m_lastKeyCode));
}

void AsKey::Key_getCode(CallArgs& ca)
{
	ca.ret = ActionValue(avm_number_t(m_lastKeyCode));
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
