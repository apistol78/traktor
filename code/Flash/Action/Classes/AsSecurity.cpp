#include "Flash/Action/Classes/AsSecurity.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/ActionContext.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsSecurity", AsSecurity, ActionClass)

Ref< AsSecurity > AsSecurity::getInstance()
{
	static AsSecurity* instance = 0;
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

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"allowDomain", createNativeFunctionValue(this, &AsSecurity::Security_allowDomain));
	prototype->setMember(L"allowInsecureDomain", createNativeFunctionValue(this, &AsSecurity::Security_allowInsecureDomain));
	prototype->setMember(L"loadPolicyFile", createNativeFunctionValue(this, &AsSecurity::Security_loadPolicyFile));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
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
