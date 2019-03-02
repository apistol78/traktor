#pragma once

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class As_flash_events_EventDispatcher : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_events_EventDispatcher(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
	void EventDispatcher_addEventListener(CallArgs& ca);

	void EventDispatcher_dispatchEvent(CallArgs& ca);

	void EventDispatcher_hasEventListener(CallArgs& ca);

	void EventDispatcher_removeEventListener(CallArgs& ca);

	void EventDispatcher_willTrigger(CallArgs& ca);
};

	}
}

