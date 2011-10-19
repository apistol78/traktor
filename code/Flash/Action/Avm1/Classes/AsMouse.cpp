#include <algorithm>
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsMouse.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsMouse", AsMouse, ActionClass)

AsMouse::AsMouse()
:	ActionClass("Mouse")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("addListener", ActionValue(createNativeFunction(this, &AsMouse::Mouse_addListener)));
	prototype->setMember("removeListener", ActionValue(createNativeFunction(this, &AsMouse::Mouse_removeListener)));
	prototype->setMember("show", ActionValue(createNativeFunction(this, &AsMouse::Mouse_show)));
	prototype->setMember("hide", ActionValue(createNativeFunction(this, &AsMouse::Mouse_hide)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

Ref< ActionObject > AsMouse::alloc(ActionContext* context)
{
	return new ActionObject("Mouse");
}

void AsMouse::init(ActionContext* context, ActionObject* self, const ActionValueArray& args)
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
		(*i)->getMember(context, "onButtonDown", member);
		if (member.isUndefined())
			continue;

		Ref< ActionFunction > eventFunction = member.getObject< ActionFunction >();
		if (eventFunction)
		{
			ActionFrame callerFrame(context, 0, 0, 4, 0, 0);
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
		(*i)->getMember(context, "onButtonUp", member);
		if (member.isUndefined())
			continue;

		Ref< ActionFunction > eventFunction = member.getObject< ActionFunction >();
		if (eventFunction)
		{
			ActionFrame callerFrame(context, 0, 0, 4, 0, 0);
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
		(*i)->getMember(context, "onMove", member);
		if (member.isUndefined())
			continue;

		Ref< ActionFunction > eventFunction = member.getObject< ActionFunction >();
		if (eventFunction)
		{
			ActionFrame callerFrame(context, 0, 0, 4, 0, 0);
			eventFunction->call(&callerFrame, (*i));
		}
	}
}

void AsMouse::removeAllListeners()
{
	m_listeners.clear();
}

void AsMouse::trace(const IVisitor& visitor) const
{
	for (RefArray< ActionObject >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		visitor(*i);
	ActionClass::trace(visitor);
}

void AsMouse::dereference()
{
	m_listeners.clear();
	ActionClass::dereference();
}

void AsMouse::Mouse_addListener(CallArgs& ca)
{
	m_listeners.push_back(ca.args[0].getObject());
}

void AsMouse::Mouse_removeListener(CallArgs& ca)
{
	RefArray< ActionObject >::iterator i = std::find(m_listeners.begin(), m_listeners.end(), ca.args[0].getObject());
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
