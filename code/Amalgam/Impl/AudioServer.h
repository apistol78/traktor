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
	bool create(const Settings* settings);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	int32_t reconfigure(const Settings* settings);

	virtual sound::SoundSystem* getSoundSystem();

	virtual sound::SurroundEnvironment* getSurroundEnvironment();

private:
	Ref< sound::SoundSystem > m_soundSystem;
	Ref< sound::SurroundEnvironment > m_surroundEnvironment;
};

	}
}

#endif	// traktor_amalgam_AudioServer_H
