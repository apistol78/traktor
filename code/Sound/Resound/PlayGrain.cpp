#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/ISoundResource.h"
#include "Sound/Sound.h"
#include "Sound/Resound/PlayGrain.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct PlayGrainCursor : public RefCountImpl< ISoundBufferCursor >
{
	Ref< ISoundBuffer > m_soundBuffer;
	Ref< ISoundBufferCursor > m_soundCursor;
	double m_timeOffset;

	virtual void setCursor(double time)
	{
		if (m_timeOffset < 0.0)
			m_timeOffset = time;

		m_soundCursor->setCursor(time - m_timeOffset);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.PlayGrain", 0, PlayGrain, IGrain)

PlayGrain::PlayGrain()
:	m_gain(0.0f)
{
}

bool PlayGrain::bind(resource::IResourceManager* resourceManager)
{
	return resourceManager->bind(m_sound);
}

Ref< ISoundBufferCursor > PlayGrain::createCursor() const
{
	if (!m_sound.validate())
		return 0;

	Ref< ISoundBuffer > soundBuffer = m_sound->getSoundBuffer();
	T_ASSERT (soundBuffer);

	Ref< ISoundBufferCursor > soundCursor = soundBuffer->createCursor();
	T_ASSERT (soundCursor);

	Ref< PlayGrainCursor > playCursor = new PlayGrainCursor();
	playCursor->m_soundBuffer = soundBuffer;
	playCursor->m_soundCursor = soundCursor;
	playCursor->m_timeOffset = -1.0;

	return playCursor;
}

bool PlayGrain::getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const
{
	PlayGrainCursor* playCursor = static_cast< PlayGrainCursor* >(cursor);
	return playCursor->m_soundBuffer->getBlock(
		playCursor->m_soundCursor,
		outBlock
	);
}

bool PlayGrain::serialize(ISerializer& s)
{
	s >> resource::Member< Sound, ISoundResource >(L"sound", m_sound);
	s >> Member< float >(L"gain", m_gain);
	return true;
}

	}
}
