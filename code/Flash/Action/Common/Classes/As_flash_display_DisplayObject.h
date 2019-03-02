#pragma once

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class As_flash_display_DisplayObject : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_display_DisplayObject(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
	void DisplayObject_getBounds(CallArgs& ca);

	void DisplayObject_getRect(CallArgs& ca);

	void DisplayObject_globalToLocal(CallArgs& ca);

	void DisplayObject_globalToLocal3D(CallArgs& ca);

	void DisplayObject_hitTestObject(CallArgs& ca);

	void DisplayObject_hitTestPoint(CallArgs& ca);

	void DisplayObject_local3DToGlobal(CallArgs& ca);

	void DisplayObject_localToGlobal(CallArgs& ca);
};

	}
}

