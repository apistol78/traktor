#include "Core/Log/Log.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsSecurity.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsSecurity", AsSecurity, ActionClass)

AsSecurity::AsSecurity()
:	ActionClass("Security")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("allowDomain", ActionValue(createNativeFunction(this, &AsSecurity::Security_allowDomain)));
	prototype->setMember("allowInsecureDomain", ActionValue(createNativeFunction(this, &AsSecurity::Security_allowInsecureDomain)));
	prototype->setMember("loadPolicyFile", ActionValue(createNativeFunction(this, &AsSecurity::Security_loadPolicyFile)));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue AsSecurity::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue();
}

void AsSecurity::Security_allowDomain(CallArgs& ca)
{
	log::warning << L"Security.allowDomain not implemented" << Endl;
}

void AsSecurity::Security_allowInsecureDomain(CallArgs& ca)
{
	log::warning << L"Security.allowDomain not implemented" << Endl;
}

void AsSecurity::Security_loadPolicyFile(CallArgs& ca)
{
	log::warning << L"Security.allowDomain not implemented" << Endl;
}

	}
}
