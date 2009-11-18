#include "Flash/Action/Classes/AsLocalConnection.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsLocalConnection", AsLocalConnection, ActionClass)

Ref< AsLocalConnection > AsLocalConnection::getInstance()
{
	static AsLocalConnection* instance = 0;
	if (!instance)
	{
		instance = new AsLocalConnection();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsLocalConnection::AsLocalConnection()
:	ActionClass(L"LocalConnection")
{
}

void AsLocalConnection::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"close", createNativeFunctionValue(this, &AsLocalConnection::LocalConnection_close));
	prototype->setMember(L"connect", createNativeFunctionValue(this, &AsLocalConnection::LocalConnection_connect));
	prototype->setMember(L"domain", createNativeFunctionValue(this, &AsLocalConnection::LocalConnection_domain));
	prototype->setMember(L"send", createNativeFunctionValue(this, &AsLocalConnection::LocalConnection_send));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsLocalConnection::construct(ActionContext* context, const args_t& args)
{
	return ActionValue::fromObject(new ActionObject(this));
}

void AsLocalConnection::LocalConnection_close(CallArgs& ca)
{
	log::warning << L"LocalConnection.close not implemented" << Endl;
}

void AsLocalConnection::LocalConnection_connect(CallArgs& ca)
{
	log::warning << L"LocalConnection.connect not implemented" << Endl;
	ca.ret = ActionValue(false);
}

void AsLocalConnection::LocalConnection_domain(CallArgs& ca)
{
	log::warning << L"LocalConnection.domain not implemented" << Endl;
}

void AsLocalConnection::LocalConnection_send(CallArgs& ca)
{
	log::warning << L"LocalConnection.send not implemented" << Endl;
	ca.ret = ActionValue(false);
}

	}
}
