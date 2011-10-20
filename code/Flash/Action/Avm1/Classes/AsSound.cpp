#include "Core/Log/Log.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsSound.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsSound", AsSound, ActionClass)

AsSound::AsSound(ActionContext* context)
:	ActionClass(context, "Sound")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("attachSound", ActionValue(createNativeFunction(context, this, &AsSound::Sound_attachSound)));
	prototype->setMember("getBytesLoaded", ActionValue(createNativeFunction(context, this, &AsSound::Sound_getBytesLoaded)));
	prototype->setMember("getBytesTotal", ActionValue(createNativeFunction(context, this, &AsSound::Sound_getBytesTotal)));
	prototype->setMember("getPan", ActionValue(createNativeFunction(context, this, &AsSound::Sound_getPan)));
	prototype->setMember("getTransform", ActionValue(createNativeFunction(context, this, &AsSound::Sound_getTransform)));
	prototype->setMember("getVolume", ActionValue(createNativeFunction(context, this, &AsSound::Sound_getVolume)));
	prototype->setMember("loadSound", ActionValue(createNativeFunction(context, this, &AsSound::Sound_loadSound)));
	prototype->setMember("setPan", ActionValue(createNativeFunction(context, this, &AsSound::Sound_setPan)));
	prototype->setMember("setTransform", ActionValue(createNativeFunction(context, this, &AsSound::Sound_setTransform)));
	prototype->setMember("setVolume", ActionValue(createNativeFunction(context, this, &AsSound::Sound_setVolume)));
	prototype->setMember("start", ActionValue(createNativeFunction(context, this, &AsSound::Sound_start)));
	prototype->setMember("stop", ActionValue(createNativeFunction(context, this, &AsSound::Sound_stop)));

	prototype->addProperty("checkPolicyFile", createNativeFunction(context, this, &AsSound::Sound_get_checkPolicyFile), createNativeFunction(context, this, &AsSound::Sound_set_checkPolicyFile));
	prototype->addProperty("duration", createNativeFunction(context, this, &AsSound::Sound_get_duration), 0);
	prototype->addProperty("id3", createNativeFunction(context, this, &AsSound::Sound_get_id3), 0);
	prototype->addProperty("position", createNativeFunction(context, this, &AsSound::Sound_get_position), 0);

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsSound::init(ActionObject* self, const ActionValueArray& args)
{
}

void AsSound::coerce(ActionObject* self) const
{
	T_FATAL_ERROR;
}

void AsSound::Sound_attachSound(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::attachSound not implemented" << Endl;
	)
}

void AsSound::Sound_getBytesLoaded(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::getBytesLoaded not implemented" << Endl;
	)
}

void AsSound::Sound_getBytesTotal(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::getBytesTotal not implemented" << Endl;
	)
}

void AsSound::Sound_getPan(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::getPan not implemented" << Endl;
	)
}

void AsSound::Sound_getTransform(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::getTransform not implemented" << Endl;
	)
}

void AsSound::Sound_getVolume(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::getVolume not implemented" << Endl;
	)
}

void AsSound::Sound_loadSound(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::loadSound not implemented" << Endl;
	)
}

void AsSound::Sound_setPan(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::setPan not implemented" << Endl;
	)
}

void AsSound::Sound_setTransform(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::setTransform not implemented" << Endl;
	)
}

void AsSound::Sound_setVolume(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::setVolume not implemented" << Endl;
	)
}

void AsSound::Sound_start(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::start not implemented" << Endl;
	)
}

void AsSound::Sound_stop(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::stop not implemented" << Endl;
	)
}

void AsSound::Sound_get_checkPolicyFile(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::get_checkPolicyFile not implemented" << Endl;
	)
}

void AsSound::Sound_set_checkPolicyFile(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::set_checkPolicyFile not implemented" << Endl;
	)
}

void AsSound::Sound_get_duration(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::get_duration not implemented" << Endl;
	)
}

void AsSound::Sound_get_id3(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::get_id3 not implemented" << Endl;
	)
}

void AsSound::Sound_get_position(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Sound::get_position not implemented" << Endl;
	)
}

	}
}
