#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/ActionValueArray.h"
#include "Flash/Action/Classes/Array.h"
#include "Flash/Action/Avm1/Classes/AsAsBroadcaster.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsAsBroadcaster", AsAsBroadcaster, ActionClass)

AsAsBroadcaster::AsAsBroadcaster(ActionContext* context)
:	ActionClass(context, "AsBroadcaster")
{
	setMember("initialize", ActionValue(createNativeFunction(context, this, &AsAsBroadcaster::AsBroadcaster_initialize)));
	setMember("addListener", ActionValue(createNativeFunction(context, this, &AsAsBroadcaster::AsBroadcaster_addListener)));
	setMember("addListenerWeak", ActionValue(createNativeFunction(context, this, &AsAsBroadcaster::AsBroadcaster_addListenerWeak)));
	setMember("broadcastMessage", ActionValue(createNativeFunction(context, this, &AsAsBroadcaster::AsBroadcaster_broadcastMessage)));
	setMember("removeListener", ActionValue(createNativeFunction(context, this, &AsAsBroadcaster::AsBroadcaster_removeListener)));

	Ref< ActionObject > prototype = new ActionObject(context);
	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsAsBroadcaster::initialize(ActionObject* self)
{
}

void AsAsBroadcaster::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue AsAsBroadcaster::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void AsAsBroadcaster::initializeSubject(ActionObject* subjectObject, bool useWeakReferences)
{
	Ref< Array > listenerArray = new Array(0);
	subjectObject->setMember("_listeners", ActionValue(listenerArray->getAsObject(getContext())));

	ActionValue functionValue;

	getMember(useWeakReferences ? "addListenerWeak" : "addListener", functionValue);
	subjectObject->setMember("addListener", functionValue);

	getMember("broadcastMessage", functionValue);
	subjectObject->setMember("broadcastMessage", functionValue);

	getMember("removeListener", functionValue);
	subjectObject->setMember("removeListener", functionValue);
}

void AsAsBroadcaster::AsBroadcaster_initialize(CallArgs& ca)
{
	Ref< ActionObject > subjectObject = ca.args[0].getObject();
	if (!subjectObject)
		return;

	initializeSubject(subjectObject, false);
}

void AsAsBroadcaster::AsBroadcaster_addListener(CallArgs& ca)
{
	ca.ret = ActionValue(false);

	if (ca.args.size() <= 0)
		return;

	if (!ca.args[0].isObject())
		return;

	ActionValue listenerValue(ca.args[0].getObject());
	if (!listenerValue.isObject())
		return;

	ActionValue listenersArrayValue;
	ca.self->getMember("_listeners", listenersArrayValue);
	if (!listenersArrayValue.isObject())
		return;

	Array* listenersArray = listenersArrayValue.getObject()->getRelay< Array >();
	if (listenersArray && listenersArray->indexOf(listenerValue) < 0)
	{
		listenersArray->push(listenerValue);
		ca.ret = ActionValue(true);
	}
}

void AsAsBroadcaster::AsBroadcaster_addListenerWeak(CallArgs& ca)
{
	ca.ret = ActionValue(false);

	if (ca.args.size() <= 0)
		return;

	if (!ca.args[0].isObject())
		return;

	ActionValue listenerValue(ca.args[0].getObject(), true);
	if (!listenerValue.isObject())
		return;

	ActionValue listenersArrayValue;
	ca.self->getMember("_listeners", listenersArrayValue);
	if (!listenersArrayValue.isObject())
		return;

	Array* listenersArray = listenersArrayValue.getObject()->getRelay< Array >();
	if (listenersArray && listenersArray->indexOf(listenerValue) < 0)
	{
		listenersArray->push(listenerValue);
		ca.ret = ActionValue(true);
	}
}

void AsAsBroadcaster::AsBroadcaster_broadcastMessage(CallArgs& ca)
{
	if (ca.args.size() < 1)
	{
		ca.ret = ActionValue(false);
		return;
	}

	std::string eventName = ca.args[0].getString();

	ActionValue listenersArrayValue;
	ca.self->getMember("_listeners", listenersArrayValue);

	if (listenersArrayValue.isObject())
	{
		ActionValueArray args(getContext()->getPool(), ca.args.size() - 1);
		for (uint32_t i = 1; i < ca.args.size(); ++i)
			args[i - 1] = ca.args[i];

		Ref< Array > listenersArray = listenersArrayValue.getObject()->getRelay< Array >();
		if (listenersArray)
		{
			AlignedVector< ActionValue > listeners = listenersArray->getValues();
			for (AlignedVector< ActionValue >::const_iterator i = listeners.begin(); i != listeners.end(); ++i)
			{
				Ref< ActionObject > listenerObject = (*i).getObject();
				if (!listenerObject)
					continue;

				ActionValue eventFunctionValue;
				listenerObject->getMember(eventName, eventFunctionValue);

				Ref< ActionFunction > eventFunction = eventFunctionValue.getObject< ActionFunction >();
				if (eventFunction)
					eventFunction->call(listenerObject, args);
			}
		}
	}

	ca.ret = ActionValue(true);
}

void AsAsBroadcaster::AsBroadcaster_removeListener(CallArgs& ca)
{
	bool result = false;

	ActionValue listenerValue;
	if (ca.args.size() > 0)
		listenerValue = ca.args[0];

	ActionValue listenersArrayValue;
	ca.self->getMember("_listeners", listenersArrayValue);

	if (listenersArrayValue.isObject())
	{
		Array* listenersArray = listenersArrayValue.getObject()->getRelay< Array >();
		if (listenersArray)
		{
			int32_t index = listenersArray->indexOf(listenerValue);
			if (index >= 0)
			{
				listenersArray->removeAt(index);
				result = true;
			}
		}
	}

	ca.ret = ActionValue(result);
}

	}
}
