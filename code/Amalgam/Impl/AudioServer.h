#pragma once

#include <string>
#include "Amalgam/IAudioServer.h"
#include "Core/Platform.h"
#include "Core/Ref.h"

namespace traktor
{

class PropertyGroup;

	namespace sound
	{

class SoundPlayer;

	}

	namespace amalgam
	{

class IEnvironment;

/*! \brief
 * \ingroup Amalgam
 */
class AudioServer : public IAudioServer
{
	T_RTTI_CLASS;

public:
	AudioServer();

	bool create(const PropertyGroup* settings, const SystemApplication& sysapp);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	void update(float dT, bool renderViewActive);

	uint32_t getActiveSoundChannels() const;

	virtual sound::SoundSystem* getSoundSystem() override final;

	virtual sound::ISoundPlayer* getSoundPlayer() override final;

	virtual sound::SurroundEnvironment* getSurroundEnvironment() override final;

private:
	Ref< sound::SoundSystem > m_soundSystem;
	Ref< sound::SoundPlayer > m_soundPlayer;
	Ref< sound::SurroundEnvironment > m_surroundEnvironment;
	std::wstring m_audioType;
	bool m_autoMute;
	bool m_soundMuted;
	float m_soundMutedVolume;
};

	}
}

