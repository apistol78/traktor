/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsSecurity.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsSecurity", AsSecurity, ActionClass)

AsSecurity::AsSecurity(ActionContext* context)
:	ActionClass(context, "Security")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("allowDomain", ActionValue(createNativeFunction(context, this, &AsSecurity::Security_allowDomain)));
	prototype->setMember("allowInsecureDomain", ActionValue(createNativeFunction(context, this, &AsSecurity::Security_allowInsecureDomain)));
	prototype->setMember("loadPolicyFile", ActionValue(createNativeFunction(context, this, &AsSecurity::Security_loadPolicyFile)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsSecurity::initialize(ActionObject* self)
{
}

void AsSecurity::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue AsSecurity::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void AsSecurity::Security_allowDomain(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Security::allowDomain not implemented" << Endl;
	)
}

void AsSecurity::Security_allowInsecureDomain(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Security::allowInsecureDomain not implemented" << Endl;
	)
}

void AsSecurity::Security_loadPolicyFile(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Security::loadPolicyFile not implemented" << Endl;
	)
}

	}
}
