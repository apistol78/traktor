#ifndef traktor_online_VoiceChat_H
#define traktor_online_VoiceChat_H

#include <list>
#include "Core/Thread/Semaphore.h"
#include "Online/IVoiceChat.h"
#include "Online/Provider/IVoiceChatProvider.h"

namespace traktor
{
	namespace sound
	{

class ISoundHandle;

	}

	namespace online
	{

class UserCache;

class VoiceChat
:	public IVoiceChat
,	public IVoiceChatProvider::IVoiceChatCallback
{
	T_RTTI_CLASS;

public:
	virtual void attachSoundPlayer(sound::ISoundPlayer* soundPlayer);

	virtual void beginTransmission(const RefArray< IUser >& audience);

	virtual void endTransmission();

	virtual void mute();

	virtual bool isTransmitting(IUser* user);

private:
	friend class SessionManager;

	struct Transmission
	{
		Ref< IUser > user;
		Ref< class VoiceSoundBuffer > soundBuffer;
		Ref< sound::ISoundHandle > soundHandle;
	};

	Semaphore m_lock;
	Ref< IVoiceChatProvider > m_provider;
	Ref< UserCache > m_userCache;
	Ref< sound::ISoundPlayer > m_soundPlayer;
	std::list< Transmission > m_transmissions;

	VoiceChat(IVoiceChatProvider* provider, UserCache* userCache);

	virtual ~VoiceChat();

	virtual void onVoiceReceived(uint64_t fromUserHandle, const int16_t* samples, uint32_t samplesCount, uint32_t sampleRate);
};

	}
}

#endif	// traktor_online_VoiceChat_H
