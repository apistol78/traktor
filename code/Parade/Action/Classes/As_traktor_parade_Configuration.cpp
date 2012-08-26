#include "Flash/Action/Classes/Array.h"
#include "Parade/Action/AsConfiguration.h"
#include "Parade/Action/AsDisplayMode.h"
#include "Parade/Action/Classes/As_traktor_parade_Configuration.h"
#include "Render/IRenderSystem.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.As_dea_parade_Configuration", As_traktor_parade_Configuration, flash::ActionClass)

As_traktor_parade_Configuration::As_traktor_parade_Configuration(flash::ActionContext* context, amalgam::IEnvironment* environment)
:	flash::ActionClass(context, "traktor.parade.Configuration")
,	m_environment(environment)
{
	// Quality enumeration.
	setMember("LOW", flash::ActionValue(flash::avm_number_t(AsConfiguration::QtLow)));
	setMember("MEDIUM", flash::ActionValue(flash::avm_number_t(AsConfiguration::QtMedium)));
	setMember("HIGH", flash::ActionValue(flash::avm_number_t(AsConfiguration::QtHigh)));
	setMember("ULTRA", flash::ActionValue(flash::avm_number_t(AsConfiguration::QtUltra)));

	// Current configuration accessor.
	addProperty("current", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_current), 0);

	Ref< flash::ActionObject > prototype = new flash::ActionObject(context);
	prototype->addProperty("displayMode", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_displayMode), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_displayMode));

	// Render
	prototype->addProperty("fullscreen", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_fullscreen), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_fullscreen));
	prototype->addProperty("waitVBlank", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_waitVBlank), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_waitVBlank));
	prototype->addProperty("multisample", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_multisample), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_multisample));
	prototype->addProperty("gamma", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_gamma), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_gamma));
	prototype->addProperty("stereoscopic", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_stereoscopic), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_stereoscopic));

	// Quality
	prototype->addProperty("textureQuality", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_textureQuality), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_textureQuality));
	prototype->addProperty("shadowQuality", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_shadowQuality), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_shadowQuality));
	prototype->addProperty("ambientOcclusionQuality", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_ambientOcclusionQuality), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_ambientOcclusionQuality));
	prototype->addProperty("terrainQuality", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_terrainQuality), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_terrainQuality));
	prototype->addProperty("waterQuality", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_waterQuality), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_waterQuality));
	prototype->addProperty("undergrowthQuality", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_undergrowthQuality), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_undergrowthQuality));

	// Audio
	prototype->addProperty("masterVolume", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_masterVolume), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_masterVolume));
	prototype->addProperty("ambientVolume", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_ambientVolume), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_ambientVolume));
	prototype->addProperty("soundFxVolume", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_soundFxVolume), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_soundFxVolume));
	prototype->addProperty("musicVolume", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_musicVolume), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_musicVolume));

	// Input
	prototype->addProperty("rumbleEnable", flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_get_rumbleEnable), flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_set_rumbleEnable));

	prototype->setMember("apply", flash::ActionValue(flash::createNativeFunction(context, this, &As_traktor_parade_Configuration::Configuration_apply)));

	prototype->setMember("constructor", flash::ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", flash::ActionValue(prototype));
}

void As_traktor_parade_Configuration::initialize(flash::ActionObject* self)
{
}

void As_traktor_parade_Configuration::construct(flash::ActionObject* self, const flash::ActionValueArray& args)
{
	self->setRelay(new AsConfiguration());
}

flash::ActionValue As_traktor_parade_Configuration::xplicit(const flash::ActionValueArray& args)
{
	return flash::ActionValue();
}

void As_traktor_parade_Configuration::Configuration_get_current(flash::CallArgs& ca)
{
	Ref< AsConfiguration > current = AsConfiguration::getCurrent(m_environment);
	if (current)
		ca.ret = flash::ActionValue(current->getAsObject(getContext()));
}

Ref< AsDisplayMode > As_traktor_parade_Configuration::Configuration_get_displayMode(const AsConfiguration* self) const
{
	return self->getDisplayMode();
}

void As_traktor_parade_Configuration::Configuration_set_displayMode(AsConfiguration* self, const AsDisplayMode* displayMode) const
{
	self->setDisplayMode(displayMode);
}

bool As_traktor_parade_Configuration::Configuration_get_fullscreen(const AsConfiguration* self) const
{
	return self->getFullscreen();
}

void As_traktor_parade_Configuration::Configuration_set_fullscreen(AsConfiguration* self, bool fullscreen) const
{
	self->setFullscreen(fullscreen);
}

bool As_traktor_parade_Configuration::Configuration_get_waitVBlank(const AsConfiguration* self) const
{
	return self->getWaitVBlank();
}

void As_traktor_parade_Configuration::Configuration_set_waitVBlank(AsConfiguration* self, bool waitVBlank) const
{
	self->setWaitVBlank(waitVBlank);
}

int32_t As_traktor_parade_Configuration::Configuration_get_multisample(const AsConfiguration* self) const
{
	return self->getMultiSample();
}

void As_traktor_parade_Configuration::Configuration_set_multisample(AsConfiguration* self, int32_t multisample) const
{
	self->setMultiSample(multisample);
}

float As_traktor_parade_Configuration::Configuration_get_gamma(const AsConfiguration* self) const
{
	return self->getGamma();
}

void As_traktor_parade_Configuration::Configuration_set_gamma(AsConfiguration* self, float gamma) const
{
	self->setGamma(gamma);
}

bool As_traktor_parade_Configuration::Configuration_get_stereoscopic(const AsConfiguration* self) const
{
	return self->getStereoscopic();
}

void As_traktor_parade_Configuration::Configuration_set_stereoscopic(AsConfiguration* self, bool stereoscopic) const
{
	self->setStereoscopic(stereoscopic);
}

int32_t As_traktor_parade_Configuration::Configuration_get_textureQuality(const AsConfiguration* self) const
{
	return self->getTextureQuality();
}

void As_traktor_parade_Configuration::Configuration_set_textureQuality(AsConfiguration* self, int32_t textureQuality) const
{
	self->setTextureQuality((AsConfiguration::Quality)textureQuality);
}

int32_t As_traktor_parade_Configuration::Configuration_get_shadowQuality(const AsConfiguration* self) const
{
	return self->getShadowQuality();
}

void As_traktor_parade_Configuration::Configuration_set_shadowQuality(AsConfiguration* self, int32_t shadowQuality) const
{
	self->setShadowQuality((AsConfiguration::Quality)shadowQuality);
}

int32_t As_traktor_parade_Configuration::Configuration_get_ambientOcclusionQuality(const AsConfiguration* self) const
{
	return self->getAmbientOcclusionQuality();
}

void As_traktor_parade_Configuration::Configuration_set_ambientOcclusionQuality(AsConfiguration* self, int32_t ambientOcclusionQuality) const
{
	self->setAmbientOcclusionQuality((AsConfiguration::Quality)ambientOcclusionQuality);
}

int32_t As_traktor_parade_Configuration::Configuration_get_terrainQuality(const AsConfiguration* self) const
{
	return 0;
}

void As_traktor_parade_Configuration::Configuration_set_terrainQuality(AsConfiguration* self, int32_t terrainQuality) const
{
}

int32_t As_traktor_parade_Configuration::Configuration_get_waterQuality(const AsConfiguration* self) const
{
	return 0;
}

void As_traktor_parade_Configuration::Configuration_set_waterQuality(AsConfiguration* self, int32_t waterQuality) const
{
}

int32_t As_traktor_parade_Configuration::Configuration_get_undergrowthQuality(const AsConfiguration* self) const
{
	return 0;
}

void As_traktor_parade_Configuration::Configuration_set_undergrowthQuality(AsConfiguration* self, int32_t undergrowthQuality) const
{
}

float As_traktor_parade_Configuration::Configuration_get_masterVolume(const AsConfiguration* self) const
{
	return self->getMasterVolume();
}

void As_traktor_parade_Configuration::Configuration_set_masterVolume(AsConfiguration* self, float masterVolume) const
{
	self->setMasterVolume(masterVolume);
}

float As_traktor_parade_Configuration::Configuration_get_ambientVolume(const AsConfiguration* self) const
{
	return self->getAmbientVolume();
}

void As_traktor_parade_Configuration::Configuration_set_ambientVolume(AsConfiguration* self, float ambientVolume) const
{
	self->setAmbientVolume(ambientVolume);
}

float As_traktor_parade_Configuration::Configuration_get_soundFxVolume(const AsConfiguration* self) const
{
	return self->getSoundFxVolume();
}

void As_traktor_parade_Configuration::Configuration_set_soundFxVolume(AsConfiguration* self, float soundFxVolume) const
{
	self->setSoundFxVolume(soundFxVolume);
}

float As_traktor_parade_Configuration::Configuration_get_musicVolume(const AsConfiguration* self) const
{
	return self->getMusicVolume();
}

void As_traktor_parade_Configuration::Configuration_set_musicVolume(AsConfiguration* self, float musicVolume) const
{
	self->setMusicVolume(musicVolume);
}

bool As_traktor_parade_Configuration::Configuration_get_rumbleEnable(const AsConfiguration* self) const
{
	return self->getRumbleEnable();
}

void As_traktor_parade_Configuration::Configuration_set_rumbleEnable(AsConfiguration* self, bool rumbleEnable) const
{
	self->setRumbleEnable(rumbleEnable);
}

bool As_traktor_parade_Configuration::Configuration_apply(AsConfiguration* self) const
{
	return self->apply(m_environment);
}

	}
}
