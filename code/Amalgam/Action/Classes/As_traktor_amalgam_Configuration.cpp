#include "Amalgam/Action/AsConfiguration.h"
#include "Amalgam/Action/AsDisplayMode.h"
#include "Amalgam/Action/AsSoundDriver.h"
#include "Amalgam/Action/Classes/As_traktor_amalgam_Configuration.h"
#include "Flash/Action/Classes/Array.h"
#include "Render/IRenderSystem.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.As_dea_amalgam_Configuration", As_traktor_amalgam_Configuration, flash::ActionClass)

As_traktor_amalgam_Configuration::As_traktor_amalgam_Configuration(flash::ActionContext* context, amalgam::IEnvironment* environment)
:	flash::ActionClass(context, "traktor.amalgam.Configuration")
,	m_environment(environment)
{
	// Quality enumeration.
	setMember("LOW", flash::ActionValue(flash::avm_number_t(AsConfiguration::QtLow)));
	setMember("MEDIUM", flash::ActionValue(flash::avm_number_t(AsConfiguration::QtMedium)));
	setMember("HIGH", flash::ActionValue(flash::avm_number_t(AsConfiguration::QtHigh)));
	setMember("ULTRA", flash::ActionValue(flash::avm_number_t(AsConfiguration::QtUltra)));

	// Current configuration accessors.
	addProperty("current", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_current), 0);

	Ref< flash::ActionObject > prototype = new flash::ActionObject(context);
	prototype->addProperty("displayMode", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_displayMode), flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_set_displayMode));

	// Render
	prototype->addProperty("fullscreen", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_fullscreen), flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_set_fullscreen));
	prototype->addProperty("waitVBlank", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_waitVBlank), flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_set_waitVBlank));
	prototype->addProperty("multisample", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_multisample), flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_set_multisample));
	prototype->addProperty("gamma", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_gamma), flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_set_gamma));
	prototype->addProperty("stereoscopic", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_stereoscopic), flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_set_stereoscopic));

	// Quality
	prototype->addProperty("textureQuality", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_textureQuality), flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_set_textureQuality));
	prototype->addProperty("shadowQuality", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_shadowQuality), flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_set_shadowQuality));
	prototype->addProperty("ambientOcclusionQuality", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_ambientOcclusionQuality), flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_set_ambientOcclusionQuality));
	prototype->addProperty("antiAliasQuality", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_antiAliasQuality), flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_set_antiAliasQuality));
	prototype->addProperty("terrainQuality", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_terrainQuality), flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_set_terrainQuality));
	prototype->addProperty("particleQuality", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_particleQuality), flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_set_particleQuality));
	prototype->addProperty("oceanQuality", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_oceanQuality), flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_set_oceanQuality));

	// Input
	prototype->addProperty("rumbleEnable", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_rumbleEnable), flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_set_rumbleEnable));

	// Audio
	prototype->addProperty("soundDriver", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_soundDriver), flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_set_soundDriver));
	prototype->addProperty("autoMute", flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_get_autoMute), flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_set_autoMute));
	prototype->setMember("getMasterVolume", flash::ActionValue(flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_getMasterVolume)));
	prototype->setMember("setMasterVolume", flash::ActionValue(flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_setMasterVolume)));
	prototype->setMember("getVolume", flash::ActionValue(flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_getVolume)));
	prototype->setMember("setVolume", flash::ActionValue(flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_setVolume)));

	// User
	prototype->setMember("getBoolean", flash::ActionValue(flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_getBoolean)));
	prototype->setMember("setBoolean", flash::ActionValue(flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_setBoolean)));
	prototype->setMember("getInteger", flash::ActionValue(flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_getInteger)));
	prototype->setMember("setInteger", flash::ActionValue(flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_setInteger)));
	prototype->setMember("getFloat", flash::ActionValue(flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_getFloat)));
	prototype->setMember("setFloat", flash::ActionValue(flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_setFloat)));

	prototype->setMember("apply", flash::ActionValue(flash::createNativeFunction(context, this, &As_traktor_amalgam_Configuration::Configuration_apply)));
	prototype->setMember("constructor", flash::ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", flash::ActionValue(prototype));
}

void As_traktor_amalgam_Configuration::initialize(flash::ActionObject* self)
{
}

void As_traktor_amalgam_Configuration::construct(flash::ActionObject* self, const flash::ActionValueArray& args)
{
	self->setRelay(new AsConfiguration());
}

flash::ActionValue As_traktor_amalgam_Configuration::xplicit(const flash::ActionValueArray& args)
{
	return flash::ActionValue();
}

void As_traktor_amalgam_Configuration::Configuration_get_current(flash::CallArgs& ca)
{
	Ref< AsConfiguration > current = AsConfiguration::getCurrent(m_environment);
	if (current)
		ca.ret = flash::ActionValue(current->getAsObject(getContext()));
}

Ref< AsDisplayMode > As_traktor_amalgam_Configuration::Configuration_get_displayMode(const AsConfiguration* self) const
{
	return self->getDisplayMode();
}

void As_traktor_amalgam_Configuration::Configuration_set_displayMode(AsConfiguration* self, const AsDisplayMode* displayMode) const
{
	self->setDisplayMode(displayMode);
}

bool As_traktor_amalgam_Configuration::Configuration_get_fullscreen(const AsConfiguration* self) const
{
	return self->getFullscreen();
}

void As_traktor_amalgam_Configuration::Configuration_set_fullscreen(AsConfiguration* self, bool fullscreen) const
{
	self->setFullscreen(fullscreen);
}

bool As_traktor_amalgam_Configuration::Configuration_get_waitVBlank(const AsConfiguration* self) const
{
	return self->getWaitVBlank();
}

void As_traktor_amalgam_Configuration::Configuration_set_waitVBlank(AsConfiguration* self, bool waitVBlank) const
{
	self->setWaitVBlank(waitVBlank);
}

int32_t As_traktor_amalgam_Configuration::Configuration_get_multisample(const AsConfiguration* self) const
{
	return self->getMultiSample();
}

void As_traktor_amalgam_Configuration::Configuration_set_multisample(AsConfiguration* self, int32_t multisample) const
{
	self->setMultiSample(multisample);
}

float As_traktor_amalgam_Configuration::Configuration_get_gamma(const AsConfiguration* self) const
{
	return self->getGamma();
}

void As_traktor_amalgam_Configuration::Configuration_set_gamma(AsConfiguration* self, float gamma) const
{
	self->setGamma(gamma);
}

bool As_traktor_amalgam_Configuration::Configuration_get_stereoscopic(const AsConfiguration* self) const
{
	return self->getStereoscopic();
}

void As_traktor_amalgam_Configuration::Configuration_set_stereoscopic(AsConfiguration* self, bool stereoscopic) const
{
	self->setStereoscopic(stereoscopic);
}

int32_t As_traktor_amalgam_Configuration::Configuration_get_textureQuality(const AsConfiguration* self) const
{
	return self->getTextureQuality();
}

void As_traktor_amalgam_Configuration::Configuration_set_textureQuality(AsConfiguration* self, int32_t textureQuality) const
{
	self->setTextureQuality((AsConfiguration::Quality)textureQuality);
}

int32_t As_traktor_amalgam_Configuration::Configuration_get_shadowQuality(const AsConfiguration* self) const
{
	return self->getShadowQuality();
}

void As_traktor_amalgam_Configuration::Configuration_set_shadowQuality(AsConfiguration* self, int32_t shadowQuality) const
{
	self->setShadowQuality((AsConfiguration::Quality)shadowQuality);
}

int32_t As_traktor_amalgam_Configuration::Configuration_get_ambientOcclusionQuality(const AsConfiguration* self) const
{
	return self->getAmbientOcclusionQuality();
}

void As_traktor_amalgam_Configuration::Configuration_set_ambientOcclusionQuality(AsConfiguration* self, int32_t ambientOcclusionQuality) const
{
	self->setAmbientOcclusionQuality((AsConfiguration::Quality)ambientOcclusionQuality);
}

int32_t As_traktor_amalgam_Configuration::Configuration_get_antiAliasQuality(const AsConfiguration* self) const
{
	return self->getAntiAliasQuality();
}

void As_traktor_amalgam_Configuration::Configuration_set_antiAliasQuality(AsConfiguration* self, int32_t antiAliasQuality) const
{
	self->setAntiAliasQuality((AsConfiguration::Quality)antiAliasQuality);
}

int32_t As_traktor_amalgam_Configuration::Configuration_get_terrainQuality(const AsConfiguration* self) const
{
	return 0;
}

void As_traktor_amalgam_Configuration::Configuration_set_terrainQuality(AsConfiguration* self, int32_t terrainQuality) const
{
}

int32_t As_traktor_amalgam_Configuration::Configuration_get_particleQuality(const AsConfiguration* self) const
{
	return self->getParticleQuality();
}

void As_traktor_amalgam_Configuration::Configuration_set_particleQuality(AsConfiguration* self, int32_t particleQuality) const
{
	self->setParticleQuality((AsConfiguration::Quality)particleQuality);
}

int32_t As_traktor_amalgam_Configuration::Configuration_get_oceanQuality(const AsConfiguration* self) const
{
	return self->getOceanQuality();
}

void As_traktor_amalgam_Configuration::Configuration_set_oceanQuality(AsConfiguration* self, int32_t oceanQuality) const
{
	self->setOceanQuality((AsConfiguration::Quality)oceanQuality);
}

bool As_traktor_amalgam_Configuration::Configuration_get_rumbleEnable(const AsConfiguration* self) const
{
	return self->getRumbleEnable();
}

void As_traktor_amalgam_Configuration::Configuration_set_rumbleEnable(AsConfiguration* self, bool rumbleEnable) const
{
	self->setRumbleEnable(rumbleEnable);
}

Ref< AsSoundDriver > As_traktor_amalgam_Configuration::Configuration_get_soundDriver(const AsConfiguration* self) const
{
	return self->getSoundDriver();
}

void As_traktor_amalgam_Configuration::Configuration_set_soundDriver(AsConfiguration* self, const AsSoundDriver* soundDriver) const
{
	self->setSoundDriver(soundDriver);
}

bool As_traktor_amalgam_Configuration::Configuration_get_autoMute(const AsConfiguration* self) const
{
	return self->getAutoMute();
}

void As_traktor_amalgam_Configuration::Configuration_set_autoMute(AsConfiguration* self, bool autoMute) const
{
	self->setAutoMute(autoMute);
}

float As_traktor_amalgam_Configuration::Configuration_getMasterVolume(const AsConfiguration* self) const
{
	return self->getVolume();
}

void As_traktor_amalgam_Configuration::Configuration_setMasterVolume(AsConfiguration* self, float volume) const
{
	self->setVolume(volume);
}

float As_traktor_amalgam_Configuration::Configuration_getVolume(const AsConfiguration* self, const std::wstring& category) const
{
	return self->getVolume(category);
}

void As_traktor_amalgam_Configuration::Configuration_setVolume(AsConfiguration* self, const std::wstring& category, float volume) const
{
	self->setVolume(category, volume);
}

bool As_traktor_amalgam_Configuration::Configuration_getBoolean(AsConfiguration* self, const std::wstring& name) const
{
	return self->getBoolean(name);
}

void As_traktor_amalgam_Configuration::Configuration_setBoolean(AsConfiguration* self, const std::wstring& name, bool value) const
{
	self->setBoolean(name, value);
}

int32_t As_traktor_amalgam_Configuration::Configuration_getInteger(AsConfiguration* self, const std::wstring& name) const
{
	return self->getInteger(name);
}

void As_traktor_amalgam_Configuration::Configuration_setInteger(AsConfiguration* self, const std::wstring& name, int32_t value) const
{
	self->setInteger(name, value);
}

float As_traktor_amalgam_Configuration::Configuration_getFloat(AsConfiguration* self, const std::wstring& name) const
{
	return self->getFloat(name);
}

void As_traktor_amalgam_Configuration::Configuration_setFloat(AsConfiguration* self, const std::wstring& name, float value) const
{
	self->setFloat(name, value);
}

bool As_traktor_amalgam_Configuration::Configuration_apply(AsConfiguration* self) const
{
	return self->apply(m_environment);
}

	}
}
