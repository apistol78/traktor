#include "Core/Log/Log.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsLocalConnection.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsLocalConnection", AsLocalConnection, ActionClass)

Ref< AsLocalConnection > AsLocalConnection::getInstance()
{
	static Ref< AsLocalConnection > instance = 0;
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

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"close", ActionValue(createNativeFunction(this, &AsLocalConnection::LocalConnection_close)));
	prototype->setMember(L"connect", ActionValue(createNativeFunction(this, &AsLocalConnection::LocalConnection_connect)));
	prototype->setMember(L"domain", ActionValue(createNativeFunction(this, &AsLocalConnection::LocalConnection_domain)));
	prototype->setMember(L"send", ActionValue(createNativeFunction(this, &AsLocalConnection::LocalConnection_send)));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsLocalConnection::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue(new ActionObject(this));
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
