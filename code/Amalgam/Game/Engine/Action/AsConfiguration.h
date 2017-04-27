/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_AsConfiguration_H
#define traktor_amalgam_AsConfiguration_H

#include <map>
#include "Amalgam/Game/IEnvironment.h"
#include "Flash/Action/ActionObjectRelay.h"
#include "Flash/Action/Common/Array.h"

namespace traktor
{

class IPropertyValue;

	namespace amalgam
	{

class AsDisplayMode;
class AsSoundDriver;

/*! \brief
 * \ingroup Amalgam
 */
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

	static Ref< AsConfiguration > getCurrent(IEnvironment* environment);

	Ref< AsDisplayMode > getDisplayMode() const;

	void setDisplayMode(const AsDisplayMode* displayMode);

	bool getFullscreen() const;

	void setFullscreen(bool fullscreen);

	int32_t getWaitVBlanks() const;

	void setWaitVBlanks(int32_t waitVBlanks);

	int32_t getMultiSample() const;

	void setMultiSample(int32_t multiSample);

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

	Quality getParticleQuality() const;

	void setParticleQuality(Quality particleQuality);

	Quality getTerrainQuality() const;

	void setTerrainQuality(Quality terrainQuality);

	Quality getOceanQuality() const;

	void setOceanQuality(Quality oceanQuality);

	Quality getImageProcessQuality() const;

	void setImageProcessQuality(Quality imageProcessQuality);

	float getGamma() const;

	void setGamma(float gamma);

	bool getRumbleEnable() const;

	void setRumbleEnable(bool rumbleEnable);

	Ref< AsSoundDriver > getSoundDriver() const;

	void setSoundDriver(const AsSoundDriver* soundDriver);

	float getVolume() const;

	void setVolume(float volume);

	float getVolume(const std::wstring& category) const;

	void setVolume(const std::wstring& category, float volume);

	bool getAutoMute() const;

	void setAutoMute(bool autoMute);

	bool getBoolean(const std::wstring& name) const;

	void setBoolean(const std::wstring& name, bool value);

	int32_t getInteger(const std::wstring& name) const;

	void setInteger(const std::wstring& name, int32_t value);

	float getFloat(const std::wstring& name) const;

	void setFloat(const std::wstring& name, float value);

	bool apply(IEnvironment* environment);

private:
	Ref< const PropertyGroup > m_settings;
	int32_t m_displayModeWidth;
	int32_t m_displayModeHeight;
	bool m_fullscreen;
	int32_t m_waitVBlanks;
	int32_t m_multiSample;
	bool m_stereoscopic;
	Quality m_textureQuality;
	Quality m_shadowQuality;
	Quality m_ambientOcclusionQuality;
	Quality m_antiAliasQuality;
	Quality m_particleQuality;
	Quality m_terrainQuality;
	Quality m_oceanQuality;
	Quality m_imageProcessQuality;
	float m_gamma;
	float m_mouseSensitivity;
	bool m_rumbleEnable;
	std::wstring m_soundDriver;
	float m_masterVolume;
	bool m_autoMute;
	std::map< std::wstring, float > m_volumes;
	std::map< std::wstring, Ref< IPropertyValue > > m_user;
};

	}
}

#endif	// traktor_amalgam_AsConfiguration_H
