#ifndef traktor_amalgam_As_traktor_amalgam_Configuration_h
#define traktor_amalgam_As_traktor_amalgam_Configuration_h

#include "Amalgam/IEnvironment.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace amalgam
	{

class AsConfiguration;
class AsDisplayMode;

class As_traktor_amalgam_Configuration : public flash::ActionClass
{
	T_RTTI_CLASS;

public:
	As_traktor_amalgam_Configuration(flash::ActionContext* context, amalgam::IEnvironment* environment);

	virtual void initialize(flash::ActionObject* self);

	virtual void construct(flash::ActionObject* self, const flash::ActionValueArray& args);

	virtual flash::ActionValue xplicit(const flash::ActionValueArray& args);

private:
	amalgam::IEnvironment* m_environment;

	void Configuration_get_current(flash::CallArgs& ca);

	Ref< AsDisplayMode > Configuration_get_displayMode(const AsConfiguration* self) const;

	void Configuration_set_displayMode(AsConfiguration* self, const AsDisplayMode* displayMode) const;

	bool Configuration_get_fullscreen(const AsConfiguration* self) const;

	void Configuration_set_fullscreen(AsConfiguration* self, bool fullscreen) const;

	bool Configuration_get_waitVBlank(const AsConfiguration* self) const;

	void Configuration_set_waitVBlank(AsConfiguration* self, bool waitVBlank) const;

	int32_t Configuration_get_multisample(const AsConfiguration* self) const;

	void Configuration_set_multisample(AsConfiguration* self, int32_t multisample) const;

	float Configuration_get_gamma(const AsConfiguration* self) const;

	void Configuration_set_gamma(AsConfiguration* self, float gamma) const;

	bool Configuration_get_stereoscopic(const AsConfiguration* self) const;

	void Configuration_set_stereoscopic(AsConfiguration* self, bool stereoscopic) const;

	int32_t Configuration_get_textureQuality(const AsConfiguration* self) const;

	void Configuration_set_textureQuality(AsConfiguration* self, int32_t textureQuality) const;

	int32_t Configuration_get_shadowQuality(const AsConfiguration* self) const;

	void Configuration_set_shadowQuality(AsConfiguration* self, int32_t shadowQuality) const;

	int32_t Configuration_get_ambientOcclusionQuality(const AsConfiguration* self) const;

	void Configuration_set_ambientOcclusionQuality(AsConfiguration* self, int32_t ambientOcclusionQuality) const;

	int32_t Configuration_get_antiAliasQuality(const AsConfiguration* self) const;

	void Configuration_set_antiAliasQuality(AsConfiguration* self, int32_t antiAliasQuality) const;

	int32_t Configuration_get_terrainQuality(const AsConfiguration* self) const;

	void Configuration_set_terrainQuality(AsConfiguration* self, int32_t terrainQuality) const;

	int32_t Configuration_get_waterQuality(const AsConfiguration* self) const;

	void Configuration_set_waterQuality(AsConfiguration* self, int32_t waterQuality) const;

	int32_t Configuration_get_undergrowthQuality(const AsConfiguration* self) const;

	void Configuration_set_undergrowthQuality(AsConfiguration* self, int32_t undergrowthQuality) const;

	bool Configuration_get_rumbleEnable(const AsConfiguration* self) const;

	void Configuration_set_rumbleEnable(AsConfiguration* self, bool rumbleEnable) const;

	float Configuration_getMasterVolume(const AsConfiguration* self) const;

	void Configuration_setMasterVolume(AsConfiguration* self, float volume) const;

	float Configuration_getVolume(const AsConfiguration* self, const std::wstring& category) const;

	void Configuration_setVolume(AsConfiguration* self, const std::wstring& category, float volume) const;

	bool Configuration_getBoolean(AsConfiguration* self, const std::wstring& name) const;

	void Configuration_setBoolean(AsConfiguration* self, const std::wstring& name, bool value) const;

	bool Configuration_apply(AsConfiguration* self) const;
};

	}
}

#endif	// traktor_amalgam_As_traktor_amalgam_Configuration_h
