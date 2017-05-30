/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Flash/Dictionary.h"
#include "Flash/SoundPlayer.h"
#include "Flash/SpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/ActionObjectRelay.h"
#include "Flash/Action/Common/Classes/AsSound.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

class SoundRelay : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	SoundRelay(const Sound* sound)
	:	ActionObjectRelay("Sound")
	,	m_sound(sound)
	{
	}

	const Sound* getSound() const { return m_sound; }

protected:
	virtual void dereference() T_OVERRIDE T_FINAL
	{
		m_sound = 0;
		ActionObjectRelay::dereference();
	}

private:
	Ref< const Sound > m_sound;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.SoundRelay", SoundRelay, ActionObjectRelay)

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsSound", AsSound, ActionClass)

AsSound::AsSound(ActionContext* context, SoundPlayer* soundPlayer)
:	ActionClass(context, "Sound")
,	m_soundPlayer(soundPlayer)
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

void AsSound::initialize(ActionObject* self)
{
}

void AsSound::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue AsSound::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void AsSound::Sound_attachSound(ActionObject* self, const std::string& exportName) const
{
	ActionContext* context = self->getContext();
	T_ASSERT (context);

	Dictionary* dictionary = context->getMovieClip()->getDictionary();
	if (!dictionary)
		return;

	uint16_t soundId;
	if (!dictionary->getExportId(exportName, soundId))
	{
		log::error << L"No sound exported as \"" << mbstows(exportName) << L"\"" << Endl;
		return;
	}

	const Sound* sound = dictionary->getSound(soundId);
	if (!sound)
	{
		log::error << L"No sound defined with id " << soundId << L", exported as \"" << mbstows(exportName) << L"\"" << Endl;
		return;
	}

	self->setRelay(new SoundRelay(sound));
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

void AsSound::Sound_start(ActionObject* self) const
{
	SoundRelay* fsr = self->getRelay< SoundRelay >();
	if (!fsr)
		return;

	m_soundPlayer->play(fsr->getSound());
}

void AsSound::Sound_stop(ActionObject* self) const
{
	SoundRelay* fsr = self->getRelay< SoundRelay >();
	if (!fsr)
		return;
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
