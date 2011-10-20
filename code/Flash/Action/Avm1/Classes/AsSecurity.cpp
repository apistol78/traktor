#include "Core/Log/Log.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsSecurity.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsSecurity", AsSecurity, ActionClass)

AsSecurity::AsSecurity(ActionContext* context)
:	ActionClass(context, "Security")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("allowDomain", ActionValue(createNativeFunction(context, this, &AsSecurity::Security_allowDomain)));
	prototype->setMember("allowInsecureDomain", ActionValue(createNativeFunction(context, this, &AsSecurity::Security_allowInsecureDomain)));
	prototype->setMember("loadPolicyFile", ActionValue(createNativeFunction(context, this, &AsSecurity::Security_loadPolicyFile)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsSecurity::init(ActionObject* self, const ActionValueArray& args) const
{
}

void AsSecurity::coerce(ActionObject* self) const
{
	T_FATAL_ERROR;
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
