/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/As_flash_display_MovieClip.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.As_flash_display_MovieClip", As_flash_display_MovieClip, ActionClass)

As_flash_display_MovieClip::As_flash_display_MovieClip(ActionContext* context)
:	ActionClass(context, "flash.display.MovieClip")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("constructor", ActionValue(this));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void As_flash_display_MovieClip::initialize(ActionObject* self)
{
}

void As_flash_display_MovieClip::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue As_flash_display_MovieClip::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

	}
}
