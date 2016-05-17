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
	void destroy();

	virtual void setSoundPlayer(sound::ISoundPlayer* soundPlayer) T_OVERRIDE T_FINAL;

	virtual void setAudience(const RefArray< IUser >& audience) T_OVERRIDE T_FINAL;

	virtual void beginTransmission() T_OVERRIDE T_FINAL;

	virtual void endTransmission() T_OVERRIDE T_FINAL;

	virtual void setMute(bool mute) T_OVERRIDE T_FINAL;

	virtual bool isTransmitting(IUser* user) T_OVERRIDE T_FINAL;

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
	uint32_t m_category;
	bool m_muted;
	std::list< Transmission > m_transmissions;

	VoiceChat(IVoiceChatProvider* provider, UserCache* userCache);

	virtual ~VoiceChat();

	virtual void onVoiceReceived(uint64_t fromUserHandle, const int16_t* samples, uint32_t samplesCount, uint32_t sampleRate) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_online_VoiceChat_H
