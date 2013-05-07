#ifndef traktor_amalgam_AsConfiguration_H
#define traktor_amalgam_AsConfiguration_H

#include <map>
#include "Amalgam/IEnvironment.h"
#include "Flash/Action/ActionObjectRelay.h"
#include "Flash/Action/Classes/Array.h"

namespace traktor
{
	namespace amalgam
	{

class AsDisplayMode;

class AsConfiguration : public flash::ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	enum Quality
	{
		QtDisabled = 0,
		QtLow = 1,
		QtMedium = 2,
		QtHigh = 3,
		QtUltra = 4
	};

	AsConfiguration();

	static Ref< AsConfiguration > getCurrent(amalgam::IEnvironment* environment);

	Ref< AsDisplayMode > getDisplayMode() const;

	void setDisplayMode(const AsDisplayMode* displayMode);

	bool getFullscreen() const;

	void setFullscreen(bool fullscreen);

	bool getWaitVBlank() const;

	void setWaitVBlank(bool waitVBlank);

	int32_t getMultiSample() const;

	void setMultiSample(int32_t multiSample);

	float getGamma() const;

	void setGamma(float gamma);

	bool getStereoscopic() const;

	void setStereoscopic(bool stereoscopic);

	Quality getTextureQuality() const;

	void setTextureQuality(Quality textureQuality);

	Quality getShadowQuality() const;

	void setShadowQuality(Quality shadowQuality);

	Quality getAmbientOcclusionQuality() const;

	void setAmbientOcclusionQuality(Quality ambientOcclusionQuality);

	Quality getAntiAliasQuality() const;

	void setAntiAliasQuality(Quality antiAliasQuality);

	bool getRumbleEnable() const;

	void setRumbleEnable(bool rumbleEnable);

	float getVolume(const std::wstring& category) const;

	void setVolume(const std::wstring& category, float volume);

	bool apply(amalgam::IEnvironment* environment);

private:
	int32_t m_displayModeWidth;
	int32_t m_displayModeHeight;
	bool m_fullscreen;
	bool m_waitVBlank;
	int32_t m_multiSample;
	float m_gamma;
	bool m_stereoscopic;
	Quality m_textureQuality;
	Quality m_shadowQuality;
	Quality m_ambientOcclusionQuality;
	Quality m_antiAliasQuality;
	bool m_rumbleEnable;
	std::map< std::wstring, float > m_volumes;
};

	}
}

#endif	// traktor_amalgam_AsConfiguration_H
