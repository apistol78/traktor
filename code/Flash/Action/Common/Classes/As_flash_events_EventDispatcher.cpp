#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/As_flash_events_EventDispatcher.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_events_EventDispatcher", As_flash_events_EventDispatcher, ActionClass)

As_flash_events_EventDispatcher::As_flash_events_EventDispatcher(ActionContext* context)
:	ActionClass(context, "flash.events.EventDispatcher")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("addEventListener", ActionValue(createNativeFunction(context, this, &As_flash_events_EventDispatcher::EventDispatcher_addEventListener)));
	prototype->setMember("dispatchEvent", ActionValue(createNativeFunction(context, this, &As_flash_events_EventDispatcher::EventDispatcher_dispatchEvent)));
	prototype->setMember("hasEventListener", ActionValue(createNativeFunction(context, this, &As_flash_events_EventDispatcher::EventDispatcher_hasEventListener)));
	prototype->setMember("removeEventListener", ActionValue(createNativeFunction(context, this, &As_flash_events_EventDispatcher::EventDispatcher_removeEventListener)));
	prototype->setMember("willTrigger", ActionValue(createNativeFunction(context, this, &As_flash_events_EventDispatcher::EventDispatcher_willTrigger)));

	prototype->setMember("constructor", ActionValue(this));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_events_EventDispatcher::initialize(ActionObject* self)
{
}

void As_flash_events_EventDispatcher::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue As_flash_events_EventDispatcher::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void As_flash_events_EventDispatcher::EventDispatcher_addEventListener(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"EventDispatcher::addEventListener not implemented" << Endl;
	);
}

void As_flash_events_EventDispatcher::EventDispatcher_dispatchEvent(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"EventDispatcher::dispatchEvent not implemented" << Endl;
	);
}

void As_flash_events_EventDispatcher::EventDispatcher_hasEventListener(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"EventDispatcher::hasEventListener not implemented" << Endl;
	);
}

void As_flash_events_EventDispatcher::EventDispatcher_removeEventListener(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"EventDispatcher::removeEventListener not implemented" << Endl;
	);
}

void As_flash_events_EventDispatcher::EventDispatcher_willTrigger(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"EventDispatcher::willTrigger not implemented" << Endl;
	);
}

	}
}
