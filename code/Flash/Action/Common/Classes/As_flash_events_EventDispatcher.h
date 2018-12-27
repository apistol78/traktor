/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_As_flash_events_EventDispatcher_H
#define traktor_flash_As_flash_events_EventDispatcher_H

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

#endif	// traktor_flash_As_flash_events_EventDispatcher_H
