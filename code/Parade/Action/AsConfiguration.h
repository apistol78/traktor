#ifndef traktor_parade_AsConfiguration_H
#define traktor_parade_AsConfiguration_H

#include "Amalgam/IEnvironment.h"
#include "Flash/Action/ActionObjectRelay.h"
#include "Flash/Action/Classes/Array.h"

namespace traktor
{
	namespace parade
	{

class AsDisplayMode;

class AsConfiguration : public flash::ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	enum Quality
	{
		QtLow = 0,
		QtMedium = 1,
		QtHigh = 2,
		QtUltra = 3
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

	float getMasterVolume() const;

	void setMasterVolume(float masterVolume);

	float getAmbientVolume() const;

	void setAmbientVolume(float ambientVolume);

	float getSoundFxVolume() const;

	void setSoundFxVolume(float soundFxVolume);

	float getMusicVolume() const;

	void setMusicVolume(float musicVolume);

	bool getRumbleEnable() const;

	void setRumbleEnable(bool rumbleEnable);

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
	float m_masterVolume;
	float m_ambientVolume;
	float m_soundFxVolume;
	float m_musicVolume;
	bool m_rumbleEnable;
};

	}
}

#endif	// traktor_parade_AsConfiguration_H
