#include "Flash/Action/Classes/AsLoadVars.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsLoadVars", AsLoadVars, ActionClass)

Ref< AsLoadVars > AsLoadVars::getInstance()
{
	static AsLoadVars* instance = 0;
	if (!instance)
	{
		instance = new AsLoadVars();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsLoadVars::AsLoadVars()
:	ActionClass(L"LoadVars")
{
}

void AsLoadVars::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"addRequestHeader", createNativeFunctionValue(this, &AsLoadVars::LoadVars_addRequestHeader));
	prototype->setMember(L"decode", createNativeFunctionValue(this, &AsLoadVars::LoadVars_decode));
	prototype->setMember(L"getBytesLoaded", createNativeFunctionValue(this, &AsLoadVars::LoadVars_getBytesLoaded));
	prototype->setMember(L"getBytesTotal", createNativeFunctionValue(this, &AsLoadVars::LoadVars_getBytesTotal));
	prototype->setMember(L"load", createNativeFunctionValue(this, &AsLoadVars::LoadVars_load));
	prototype->setMember(L"send", createNativeFunctionValue(this, &AsLoadVars::LoadVars_send));
	prototype->setMember(L"sendAndLoad", createNativeFunctionValue(this, &AsLoadVars::LoadVars_sendAndLoad));
	prototype->setMember(L"toString", createNativeFunctionValue(this, &AsLoadVars::LoadVars_toString));

	prototype->addProperty(L"contentType", createNativeFunction(this, &AsLoadVars::LoadVars_get_contentType), createNativeFunction(this, &AsLoadVars::LoadVars_set_contentType));
	prototype->addProperty(L"loaded", createNativeFunction(this, &AsLoadVars::LoadVars_get_loaded), createNativeFunction(this, &AsLoadVars::LoadVars_set_loaded));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsLoadVars::construct(ActionContext* context, const args_t& args)
{
	return ActionValue::fromObject(new ActionObject(this));
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
