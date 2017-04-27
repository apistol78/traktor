/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsLoadVars.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsLoadVars", AsLoadVars, ActionClass)

AsLoadVars::AsLoadVars(ActionContext* context)
:	ActionClass(context, "LoadVars")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("addRequestHeader", ActionValue(createNativeFunction(context, this, &AsLoadVars::LoadVars_addRequestHeader)));
	prototype->setMember("decode", ActionValue(createNativeFunction(context, this, &AsLoadVars::LoadVars_decode)));
	prototype->setMember("getBytesLoaded", ActionValue(createNativeFunction(context, this, &AsLoadVars::LoadVars_getBytesLoaded)));
	prototype->setMember("getBytesTotal", ActionValue(createNativeFunction(context, this, &AsLoadVars::LoadVars_getBytesTotal)));
	prototype->setMember("load", ActionValue(createNativeFunction(context, this, &AsLoadVars::LoadVars_load)));
	prototype->setMember("send", ActionValue(createNativeFunction(context, this, &AsLoadVars::LoadVars_send)));
	prototype->setMember("sendAndLoad", ActionValue(createNativeFunction(context, this, &AsLoadVars::LoadVars_sendAndLoad)));
	prototype->setMember("toString", ActionValue(createNativeFunction(context, this, &AsLoadVars::LoadVars_toString)));

	prototype->addProperty("contentType", createNativeFunction(context, this, &AsLoadVars::LoadVars_get_contentType), createNativeFunction(context, this, &AsLoadVars::LoadVars_set_contentType));
	prototype->addProperty("loaded", createNativeFunction(context, this, &AsLoadVars::LoadVars_get_loaded), createNativeFunction(context, this, &AsLoadVars::LoadVars_set_loaded));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsLoadVars::initialize(ActionObject* self)
{
}

void AsLoadVars::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue AsLoadVars::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void AsLoadVars::LoadVars_addRequestHeader(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"LoadVars::addRequestHeader not implemented" << Endl;
	)
}

void AsLoadVars::LoadVars_decode(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"LoadVars::decode not implemented" << Endl;
	)
}

void AsLoadVars::LoadVars_getBytesLoaded(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"LoadVars::getBytesLoaded not implemented" << Endl;
	)
}

void AsLoadVars::LoadVars_getBytesTotal(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"LoadVars::getBytesTotal not implemented" << Endl;
	)
}

void AsLoadVars::LoadVars_load(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"LoadVars::load not implemented" << Endl;
	)
}

void AsLoadVars::LoadVars_send(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"LoadVars::send not implemented" << Endl;
	)
}

void AsLoadVars::LoadVars_sendAndLoad(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"LoadVars::sendAndLoad not implemented" << Endl;
	)
}

void AsLoadVars::LoadVars_toString(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"LoadVars::toString not implemented" << Endl;
	)
}

void AsLoadVars::LoadVars_get_contentType(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"LoadVars::get_contentType not implemented" << Endl;
	)
}

void AsLoadVars::LoadVars_set_contentType(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"LoadVars::set_contentType not implemented" << Endl;
	)
}

void AsLoadVars::LoadVars_get_loaded(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"LoadVars::get_loaded not implemented" << Endl;
	)
}

void AsLoadVars::LoadVars_set_loaded(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"LoadVars::set_loaded not implemented" << Endl;
	)
}

	}
}
