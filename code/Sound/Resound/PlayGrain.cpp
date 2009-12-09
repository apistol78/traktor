#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Sound.h"
#include "Sound/Resound/BankBuffer.h"
#include "Sound/Resound/BankSound.h"
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
	double m_pitch;
	float m_blocks[SbcMaxChannelCount][4096];

	virtual void setCursor(double time)
	{
		if (m_timeOffset < 0.0)
			m_timeOffset = time;

		m_soundCursor->setCursor((time - m_timeOffset) * m_pitch);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.PlayGrain", 0, PlayGrain, IGrain)

PlayGrain::PlayGrain()
:	m_index(0)
{
}

PlayGrain::PlayGrain(int32_t index)
:	m_index(index)
{
}

Ref< ISoundBufferCursor > PlayGrain::createCursor(const BankBuffer* bank) const
{
	Ref< BankSound > bankSound = bank->getSound(m_index);
	if (!bankSound || !bankSound->getSound().validate())
		return 0;

	Ref< ISoundBuffer > soundBuffer = bankSound->getSound()->getSoundBuffer();
	T_ASSERT (soundBuffer);

	Ref< ISoundBufferCursor > soundCursor = soundBuffer->createCursor();
	T_ASSERT (soundCursor);

	Ref< PlayGrainCursor > playCursor = new PlayGrainCursor();
	playCursor->m_soundBuffer = soundBuffer;
	playCursor->m_soundCursor = soundCursor;
	playCursor->m_timeOffset = -1.0;
	playCursor->m_pitch = bankSound->getPitch();

	return playCursor;
}

bool PlayGrain::getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const
{
	PlayGrainCursor* playCursor = static_cast< PlayGrainCursor* >(cursor);
	if (!playCursor->m_soundBuffer->getBlock(
		playCursor->m_soundCursor,
		outBlock
	))
		return false;

	if (abs(playCursor->m_pitch - 1.0) > 0.01)
	{
		uint32_t samplesCount = uint32_t(outBlock.samplesCount / playCursor->m_pitch);
		for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
		{
			for (uint32_t j = 0; j < samplesCount; ++j)
				playCursor->m_blocks[i][j] = outBlock.samples[i][uint32_t(j * playCursor->m_pitch)];

			outBlock.samples[i] = playCursor->m_blocks[i];
		}
		outBlock.samplesCount = samplesCount;
	}

	return true;
}

bool PlayGrain::serialize(ISerializer& s)
{
	return s >> Member< int32_t >(L"index", m_index);
}

	}
}
