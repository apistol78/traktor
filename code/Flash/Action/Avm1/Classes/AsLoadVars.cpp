#include "Core/Log/Log.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsLoadVars.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsLoadVars", AsLoadVars, ActionClass)

AsLoadVars::AsLoadVars()
:	ActionClass("LoadVars")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("addRequestHeader", ActionValue(createNativeFunction(this, &AsLoadVars::LoadVars_addRequestHeader)));
	prototype->setMember("decode", ActionValue(createNativeFunction(this, &AsLoadVars::LoadVars_decode)));
	prototype->setMember("getBytesLoaded", ActionValue(createNativeFunction(this, &AsLoadVars::LoadVars_getBytesLoaded)));
	prototype->setMember("getBytesTotal", ActionValue(createNativeFunction(this, &AsLoadVars::LoadVars_getBytesTotal)));
	prototype->setMember("load", ActionValue(createNativeFunction(this, &AsLoadVars::LoadVars_load)));
	prototype->setMember("send", ActionValue(createNativeFunction(this, &AsLoadVars::LoadVars_send)));
	prototype->setMember("sendAndLoad", ActionValue(createNativeFunction(this, &AsLoadVars::LoadVars_sendAndLoad)));
	prototype->setMember("toString", ActionValue(createNativeFunction(this, &AsLoadVars::LoadVars_toString)));

	prototype->addProperty("contentType", createNativeFunction(this, &AsLoadVars::LoadVars_get_contentType), createNativeFunction(this, &AsLoadVars::LoadVars_set_contentType));
	prototype->addProperty("loaded", createNativeFunction(this, &AsLoadVars::LoadVars_get_loaded), createNativeFunction(this, &AsLoadVars::LoadVars_set_loaded));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue AsLoadVars::construct(ActionContext* context, const ActionValueArray& args)
{
	return ActionValue(new ActionObject("LoadVars"));
}

void AsLoadVars::LoadVars_addRequestHeader(CallArgs& ca)
{
	log::warning << L"LoadVars.addRequestHeader not implemented" << Endl;
}

void AsLoadVars::LoadVars_decode(CallArgs& ca)
{
	log::warning << L"LoadVars.decode not implemented" << Endl;
}

void AsLoadVars::LoadVars_getBytesLoaded(CallArgs& ca)
{
	log::warning << L"LoadVars.getBytesLoaded not implemented" << Endl;
}

void AsLoadVars::LoadVars_getBytesTotal(CallArgs& ca)
{
	log::warning << L"LoadVars.getBytesTotal not implemented" << Endl;
}

void AsLoadVars::LoadVars_load(CallArgs& ca)
{
	log::warning << L"LoadVars.load not implemented" << Endl;
}

void AsLoadVars::LoadVars_send(CallArgs& ca)
{
	log::warning << L"LoadVars.send not implemented" << Endl;
}

void AsLoadVars::LoadVars_sendAndLoad(CallArgs& ca)
{
	log::warning << L"LoadVars.sendAndLoad not implemented" << Endl;
}

void AsLoadVars::LoadVars_toString(CallArgs& ca)
{
	log::warning << L"LoadVars.toString not implemented" << Endl;
}

void AsLoadVars::LoadVars_get_contentType(CallArgs& ca)
{
	log::warning << L"LoadVars.contentType (get) not implemented" << Endl;
}

void AsLoadVars::LoadVars_set_contentType(CallArgs& ca)
{
	log::warning << L"LoadVars.contentType (set) not implemented" << Endl;
}

void AsLoadVars::LoadVars_get_loaded(CallArgs& ca)
{
	log::warning << L"LoadVars.loaded (get) not implemented" << Endl;
}

void AsLoadVars::LoadVars_set_loaded(CallArgs& ca)
{
	log::warning << L"LoadVars.loaded (set) not implemented" << Endl;
}

	}
}
