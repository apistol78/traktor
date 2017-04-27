/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsSystem.h"
#include "Flash/Action/Common/Classes/AsSecurity.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsSystem", AsSystem, ActionObject)

AsSystem::AsSystem(ActionContext* context)
:	ActionObject(context)
{
	setMember("security", ActionValue(new AsSecurity(context)));
}

	}
}
