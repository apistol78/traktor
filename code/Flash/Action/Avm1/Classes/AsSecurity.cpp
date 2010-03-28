#include "Core/Log/Log.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsSecurity.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsSecurity", AsSecurity, ActionClass)

Ref< AsSecurity > AsSecurity::getInstance()
{
	static Ref< AsSecurity > instance = 0;
	if (!instance)
	{
		instance = new AsSecurity();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsSecurity::AsSecurity()
:	ActionClass(L"Security")
{
}

void AsSecurity::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"allowDomain", ActionValue(createNativeFunction(this, &AsSecurity::Security_allowDomain)));
	prototype->setMember(L"allowInsecureDomain", ActionValue(createNativeFunction(this, &AsSecurity::Security_allowInsecureDomain)));
	prototype->setMember(L"loadPolicyFile", ActionValue(createNativeFunction(this, &AsSecurity::Security_loadPolicyFile)));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsSecurity::construct(ActionContext* context, const args_t& args)
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
