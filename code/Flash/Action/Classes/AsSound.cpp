#include "Flash/Action/Classes/AsSound.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/ActionContext.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsSound", AsSound, ActionClass)

AsSound* AsSound::getInstance()
{
	static AsSound* instance = 0;
	if (!instance)
	{
		instance = new AsSound();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsSound::AsSound()
:	ActionClass(L"Sound")
{
}

void AsSound::createPrototype()
{
	Ref< ActionObject > prototype = gc_new< ActionObject >();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"attachSound", createNativeFunctionValue(this, &AsSound::Sound_attachSound));
	prototype->setMember(L"getBytesLoaded", createNativeFunctionValue(this, &AsSound::Sound_getBytesLoaded));
	prototype->setMember(L"getBytesTotal", createNativeFunctionValue(this, &AsSound::Sound_getBytesTotal));
	prototype->setMember(L"getPan", createNativeFunctionValue(this, &AsSound::Sound_getPan));
	prototype->setMember(L"getTransform", createNativeFunctionValue(this, &AsSound::Sound_getTransform));
	prototype->setMember(L"getVolume", createNativeFunctionValue(this, &AsSound::Sound_getVolume));
	prototype->setMember(L"loadSound", createNativeFunctionValue(this, &AsSound::Sound_loadSound));
	prototype->setMember(L"setPan", createNativeFunctionValue(this, &AsSound::Sound_setPan));
	prototype->setMember(L"setTransform", createNativeFunctionValue(this, &AsSound::Sound_setTransform));
	prototype->setMember(L"setVolume", createNativeFunctionValue(this, &AsSound::Sound_setVolume));
	prototype->setMember(L"start", createNativeFunctionValue(this, &AsSound::Sound_start));
	prototype->setMember(L"stop", createNativeFunctionValue(this, &AsSound::Sound_stop));

	prototype->addProperty(L"checkPolicyFile", createNativeFunction(this, &AsSound::Sound_get_checkPolicyFile), createNativeFunction(this, &AsSound::Sound_set_checkPolicyFile));
	prototype->addProperty(L"duration", createNativeFunction(this, &AsSound::Sound_get_duration), 0);
	prototype->addProperty(L"id3", createNativeFunction(this, &AsSound::Sound_get_id3), 0);
	prototype->addProperty(L"position", createNativeFunction(this, &AsSound::Sound_get_position), 0);

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsSound::construct(ActionContext* context, const args_t& args)
{
	return ActionValue();
}

void AsSound::Sound_attachSound(CallArgs& ca)
{
	log::warning << L"Sound.attachSound not implemented" << Endl;
}

void AsSound::Sound_getBytesLoaded(CallArgs& ca)
{
	log::warning << L"Sound.getBytesLoaded not implemented" << Endl;
	ca.ret = ActionValue(0.0);
}

void AsSound::Sound_getBytesTotal(CallArgs& ca)
{
	log::warning << L"Sound.getBytesTotal not implemented" << Endl;
	ca.ret = ActionValue(0.0);
}

void AsSound::Sound_getPan(CallArgs& ca)
{
	log::warning << L"Sound.getPan not implemented" << Endl;
	ca.ret = ActionValue(0.0);
}

void AsSound::Sound_getTransform(CallArgs& ca)
{
	log::warning << L"Sound.getTransform not implemented" << Endl;
}

void AsSound::Sound_getVolume(CallArgs& ca)
{
	log::warning << L"Sound.getVolume not implemented" << Endl;
	ca.ret = ActionValue(100.0);
}

void AsSound::Sound_loadSound(CallArgs& ca)
{
	log::warning << L"Sound.loadSound not implemented" << Endl;
}

void AsSound::Sound_setPan(CallArgs& ca)
{
	log::warning << L"Sound.setPan not implemented" << Endl;
}

void AsSound::Sound_setTransform(CallArgs& ca)
{
	log::warning << L"Sound.setTransform not implemented" << Endl;
}

void AsSound::Sound_setVolume(CallArgs& ca)
{
	log::warning << L"Sound.setVolume not implemented" << Endl;
}

void AsSound::Sound_start(CallArgs& ca)
{
	log::warning << L"Sound.start not implemented" << Endl;
}

void AsSound::Sound_stop(CallArgs& ca)
{
	log::warning << L"Sound.stop not implemented" << Endl;
}

void AsSound::Sound_get_checkPolicyFile(CallArgs& ca)
{
	ca.ret = ActionValue(false);
}

void AsSound::Sound_set_checkPolicyFile(CallArgs& ca)
{
}

void AsSound::Sound_get_duration(CallArgs& ca)
{
	ca.ret = ActionValue(0.0);
}

void AsSound::Sound_get_id3(CallArgs& ca)
{
	ca.ret = ActionValue("Not implemented");
}

void AsSound::Sound_get_position(CallArgs& ca)
{
	ca.ret = ActionValue(0.0);
}

	}
}
