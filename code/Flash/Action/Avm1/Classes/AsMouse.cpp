#include <algorithm>
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsMouse.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsMouse", AsMouse, ActionClass)

Ref< AsMouse > AsMouse::createInstance()
{
	Ref< AsMouse > instance = new AsMouse();
	instance->createPrototype();
	instance->setReadOnly();
	return instance;
}

AsMouse::AsMouse()
:	ActionClass(L"Mouse")
{
}

void AsMouse::eventMouseDown(ActionContext* context, int x, int y, int button)
{
	// Create a snapshot of active listeners the moment this event is raised,
	// this because listeners can be either added or removed by listeners.
	RefArray< ActionObject > listeners = m_listeners;
	for (RefArray< ActionObject >::iterator i = listeners.begin(); i != listeners.end(); ++i)
	{
		ActionValue member;
		(*i)->getMember(L"onButtonDown", member);
		if (member.isUndefined())
			continue;

		Ref< ActionFunction > eventFunction = member.getObject< ActionFunction >();
		if (eventFunction)
		{
			ActionFrame callerFrame(context, 0, 0, 0, 4, 0, 0);
			eventFunction->call(&callerFrame, (*i));
		}
	}
}

void AsMouse::eventMouseUp(ActionContext* context, int x, int y, int button)
{
	// Create a snapshot of active listeners the moment this event is raised,
	// this because listeners can be either added or removed by listeners.
	RefArray< ActionObject > listeners = m_listeners;
	for (RefArray< ActionObject >::iterator i = listeners.begin(); i != listeners.end(); ++i)
	{
		ActionValue member;
		(*i)->getMember(L"onButtonUp", member);
		if (member.isUndefined())
			continue;

		Ref< ActionFunction > eventFunction = member.getObject< ActionFunction >();
		if (eventFunction)
		{
			ActionFrame callerFrame(context, 0, 0, 0, 4, 0, 0);
			eventFunction->call(&callerFrame, (*i));
		}
	}
}

void AsMouse::eventMouseMove(ActionContext* context, int x, int y, int button)
{
	// Create a snapshot of active listeners the moment this event is raised,
	// this because listeners can be either added or removed by listeners.
	RefArray< ActionObject > listeners = m_listeners;
	for (RefArray< ActionObject >::iterator i = listeners.begin(); i != listeners.end(); ++i)
	{
		ActionValue member;
		(*i)->getMember(L"onMove", member);
		if (member.isUndefined())
			continue;

		Ref< ActionFunction > eventFunction = member.getObject< ActionFunction >();
		if (eventFunction)
		{
			ActionFrame callerFrame(context, 0, 0, 0, 4, 0, 0);
			eventFunction->call(&callerFrame, (*i));
		}
	}
}

void AsMouse::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"addListener", ActionValue(createNativeFunction(this, &AsMouse::Mouse_addListener)));
	prototype->setMember(L"removeListener", ActionValue(createNativeFunction(this, &AsMouse::Mouse_removeListener)));
	prototype->setMember(L"show", ActionValue(createNativeFunction(this, &AsMouse::Mouse_show)));
	prototype->setMember(L"hide", ActionValue(createNativeFunction(this, &AsMouse::Mouse_hide)));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsMouse::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue();
}

void AsMouse::Mouse_addListener(CallArgs& ca)
{
	m_listeners.push_back(ca.args[0].getObjectSafe());
}

void AsMouse::Mouse_removeListener(CallArgs& ca)
{
	RefArray< ActionObject >::iterator i = std::find(m_listeners.begin(), m_listeners.end(), ca.args[0].getObjectSafe());
	if (i != m_listeners.end())
		m_listeners.erase(i);
}

void AsMouse::Mouse_show(CallArgs& ca)
{
}

void AsMouse::Mouse_hide(CallArgs& ca)
{
}

	}
}
