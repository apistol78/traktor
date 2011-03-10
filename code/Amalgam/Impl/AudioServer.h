#ifndef traktor_amalgam_AudioServer_H
#define traktor_amalgam_AudioServer_H

#include "Amalgam/IAudioServer.h"

namespace traktor
{

class Settings;

	namespace amalgam
	{

class IEnvironment;

class AudioServer : public IAudioServer
{
	T_RTTI_CLASS;

public:
	AudioServer();

	bool create(const Settings* settings);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	int32_t reconfigure(const Settings* settings);

	void update(float dT, bool renderViewActive);

	virtual sound::SoundSystem* getSoundSystem();

	virtual sound::SurroundEnvironment* getSurroundEnvironment();

private:
	Ref< sound::SoundSystem > m_soundSystem;
	Ref< sound::SurroundEnvironment > m_surroundEnvironment;
	bool m_soundMuted;
	float m_soundMutedVolume;
};

	}
}

#endif	// traktor_amalgam_AudioServer_H
