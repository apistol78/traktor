/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/As_flash_display_DisplayObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_display_DisplayObject", As_flash_display_DisplayObject, ActionClass)

As_flash_display_DisplayObject::As_flash_display_DisplayObject(ActionContext* context)
:	ActionClass(context, "flash.display.DisplayObject")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("getBounds", ActionValue(createNativeFunction(context, this, &As_flash_display_DisplayObject::DisplayObject_getBounds)));
	prototype->setMember("getRect", ActionValue(createNativeFunction(context, this, &As_flash_display_DisplayObject::DisplayObject_getRect)));
	prototype->setMember("globalToLocal", ActionValue(createNativeFunction(context, this, &As_flash_display_DisplayObject::DisplayObject_globalToLocal)));
	prototype->setMember("globalToLocal3D", ActionValue(createNativeFunction(context, this, &As_flash_display_DisplayObject::DisplayObject_globalToLocal3D)));
	prototype->setMember("hitTestObject", ActionValue(createNativeFunction(context, this, &As_flash_display_DisplayObject::DisplayObject_hitTestObject)));
	prototype->setMember("hitTestPoint", ActionValue(createNativeFunction(context, this, &As_flash_display_DisplayObject::DisplayObject_hitTestPoint)));
	prototype->setMember("local3DToGlobal", ActionValue(createNativeFunction(context, this, &As_flash_display_DisplayObject::DisplayObject_local3DToGlobal)));
	prototype->setMember("localToGlobal", ActionValue(createNativeFunction(context, this, &As_flash_display_DisplayObject::DisplayObject_localToGlobal)));

	prototype->setMember("constructor", ActionValue(this));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_display_DisplayObject::initialize(ActionObject* self)
{
}

void As_flash_display_DisplayObject::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue As_flash_display_DisplayObject::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void As_flash_display_DisplayObject::DisplayObject_getBounds(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"DisplayObject::getBounds not implemented" << Endl;
	);
}

void As_flash_display_DisplayObject::DisplayObject_getRect(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"DisplayObject::getRect not implemented" << Endl;
	);
}

void As_flash_display_DisplayObject::DisplayObject_globalToLocal(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"DisplayObject::globalToLocal not implemented" << Endl;
	);
}

void As_flash_display_DisplayObject::DisplayObject_globalToLocal3D(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"DisplayObject::globalToLocal3D not implemented" << Endl;
	);
}

void As_flash_display_DisplayObject::DisplayObject_hitTestObject(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"DisplayObject::hitTestObject not implemented" << Endl;
	);
}

void As_flash_display_DisplayObject::DisplayObject_hitTestPoint(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"DisplayObject::hitTestPoint not implemented" << Endl;
	);
}

void As_flash_display_DisplayObject::DisplayObject_local3DToGlobal(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"DisplayObject::local3DToGlobal not implemented" << Endl;
	);
}

void As_flash_display_DisplayObject::DisplayObject_localToGlobal(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"DisplayObject::localToGlobal not implemented" << Endl;
	);
}

	}
}
