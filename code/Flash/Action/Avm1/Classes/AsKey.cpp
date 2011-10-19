#include <algorithm>
#include <cstring>
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsKey.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsKey", AsKey, ActionClass)

AsKey::AsKey()
:	ActionClass("Key")
,	m_lastKeyCode(0)
{
	std::memset(m_keyState, 0, sizeof(m_keyState));

	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("BACKSPACE", ActionValue(avm_number_t(AkBackspace)));
	prototype->setMember("CAPSLOCK", ActionValue(avm_number_t(AkCapsLock)));
	prototype->setMember("CONTROL", ActionValue(avm_number_t(AkControl)));
	prototype->setMember("DELETEKEY", ActionValue(avm_number_t(AkDeleteKey)));
	prototype->setMember("DOWN", ActionValue(avm_number_t(AkDown)));
	prototype->setMember("END", ActionValue(avm_number_t(AkEnd)));
	prototype->setMember("ENTER", ActionValue(avm_number_t(AkEnter)));
	prototype->setMember("ESCAPE", ActionValue(avm_number_t(AkEscape)));
	prototype->setMember("HOME", ActionValue(avm_number_t(AkHome)));
	prototype->setMember("INSERT", ActionValue(avm_number_t(AkInsert)));
	prototype->setMember("LEFT", ActionValue(avm_number_t(AkLeft)));
	prototype->setMember("PGDN", ActionValue(avm_number_t(AkPgDn)));
	prototype->setMember("PGUP", ActionValue(avm_number_t(AkPgUp)));
	prototype->setMember("RIGHT", ActionValue(avm_number_t(AkRight)));
	prototype->setMember("SHIFT", ActionValue(avm_number_t(AkShift)));
	prototype->setMember("SPACE", ActionValue(avm_number_t(AkSpace)));
	prototype->setMember("TAB", ActionValue(avm_number_t(AkTab)));
	prototype->setMember("UP", ActionValue(avm_number_t(AkUp)));
	prototype->setMember("addListener", ActionValue(createNativeFunction(this, &AsKey::Key_addListener)));
	prototype->setMember("getAscii", ActionValue(createNativeFunction(this, &AsKey::Key_getAscii)));
	prototype->setMember("getCode", ActionValue(createNativeFunction(this, &AsKey::Key_getCode)));
	prototype->setMember("isAccessible", ActionValue(createNativeFunction(this, &AsKey::Key_isAccessible)));
	prototype->setMember("isDown", ActionValue(createNativeFunction(this, &AsKey::Key_isDown)));
	prototype->setMember("isToggled", ActionValue(createNativeFunction(this, &AsKey::Key_isToggled)));
	prototype->setMember("removeListener", ActionValue(createNativeFunction(this, &AsKey::Key_removeListener)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

Ref< ActionObject > AsKey::alloc(ActionContext* context)
{
	return new ActionObject("Key");
}

void AsKey::init(ActionContext* context, ActionObject* self, const ActionValueArray& args)
{
}

void AsKey::eventKeyDown(ActionContext* context, int keyCode)
{
	m_keyState[keyCode] = true;
	m_lastKeyCode = keyCode;

	// Create a snapshot of active listeners the moment this event is raised,
	// this because listeners can be either added or removed by listeners.
	RefArray< ActionObject > listeners = m_listeners;
	for (RefArray< ActionObject >::iterator i = listeners.begin(); i != listeners.end(); ++i)
	{
		ActionValue member;
		(*i)->getMember(context, "onKeyDown", member);
		if (member.isUndefined())
			continue;

		Ref< ActionFunction > eventFunction = checked_type_cast< ActionFunction* >(member.getObject());
		if (eventFunction)
		{
			ActionFrame callerFrame(context, 0, 0, 4, 0, 0);
			eventFunction->call(&callerFrame, (*i));
		}
	}
}

void AsKey::eventKeyUp(ActionContext* context, int keyCode)
{
	m_keyState[keyCode] = false;

	// Create a snapshot of active listeners the moment this event is raised,
	// this because listeners can be either added or removed by listeners.
	RefArray< ActionObject > listeners = m_listeners;
	for (RefArray< ActionObject >::iterator i = listeners.begin(); i != listeners.end(); ++i)
	{
		ActionValue member;
		(*i)->getMember(context, "onKeyUp", member);
		if (member.isUndefined())
			continue;

		Ref< ActionFunction > eventFunction = checked_type_cast< ActionFunction* >(member.getObject());
		if (eventFunction)
		{
			ActionFrame callerFrame(context, 0, 0, 4, 0, 0);
			eventFunction->call(&callerFrame, (*i));
		}
	}
}

void AsKey::removeAllListeners()
{
	m_listeners.clear();
}

void AsKey::trace(const IVisitor& visitor) const
{
	for (RefArray< ActionObject >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		visitor(*i);
	ActionClass::trace(visitor);
}

void AsKey::dereference()
{
	m_listeners.clear();
	ActionClass::dereference();
}

void AsKey::Key_addListener(CallArgs& ca)
{
	m_listeners.push_back(ca.args[0].getObject());
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
	RefArray< ActionObject >::iterator i = std::find(m_listeners.begin(), m_listeners.end(), ca.args[0].getObject());
	if (i != m_listeners.end())
		m_listeners.erase(i);
}

	}
}
