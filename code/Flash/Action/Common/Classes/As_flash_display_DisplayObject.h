/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_As_flash_display_DisplayObject_H
#define traktor_flash_As_flash_display_DisplayObject_H

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

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;

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

#endif	// traktor_flash_As_flash_display_DisplayObject_H
