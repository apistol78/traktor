/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsLocalConnection.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsLocalConnection", AsLocalConnection, ActionClass)

AsLocalConnection::AsLocalConnection(ActionContext* context)
:	ActionClass(context, "LocalConnection")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("close", ActionValue(createNativeFunction(context, this, &AsLocalConnection::LocalConnection_close)));
	prototype->setMember("connect", ActionValue(createNativeFunction(context, this, &AsLocalConnection::LocalConnection_connect)));
	prototype->setMember("domain", ActionValue(createNativeFunction(context, this, &AsLocalConnection::LocalConnection_domain)));
	prototype->setMember("send", ActionValue(createNativeFunction(context, this, &AsLocalConnection::LocalConnection_send)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsLocalConnection::initialize(ActionObject* self)
{
}

void AsLocalConnection::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue AsLocalConnection::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void AsLocalConnection::LocalConnection_close(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"LocalConnection::close not implemented" << Endl;
	)
}

void AsLocalConnection::LocalConnection_connect(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"LocalConnection::connect not implemented" << Endl;
	)
}

void AsLocalConnection::LocalConnection_domain(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"LocalConnection::domain not implemented" << Endl;
	)
}

void AsLocalConnection::LocalConnection_send(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"LocalConnection::send not implemented" << Endl;
	)
}

	}
}
