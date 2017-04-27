/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsAccessibility.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsAccessibility", AsAccessibility, ActionObject)

AsAccessibility::AsAccessibility(ActionContext* context)
:	ActionObject(context)
{
	setMember("isActive", ActionValue(createNativeFunction(context, this, &AsAccessibility::Accessibility_isActive)));
	setMember("updateProperties", ActionValue(createNativeFunction(context, this, &AsAccessibility::Accessibility_updateProperties)));
}

void AsAccessibility::Accessibility_isActive(CallArgs& ca)
{
	ca.ret = ActionValue(false);
}

void AsAccessibility::Accessibility_updateProperties(CallArgs& ca)
{
}

	}
}
