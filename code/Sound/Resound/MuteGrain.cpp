#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Timer/Timer.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/Resound/MuteGrain.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct MuteGrainCursor : public RefCountImpl< ISoundBufferCursor >
{
	Timer m_timer;
	double m_end;

	virtual void reset() {}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.MuteGrain", 0, MuteGrain, IGrain)

MuteGrain::MuteGrain()
:	m_duration(0.0)
{
}

bool MuteGrain::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< ISoundBufferCursor > MuteGrain::createCursor() const
{
	Ref< MuteGrainCursor > muteCursor = new MuteGrainCursor();
	muteCursor->m_timer.start();
	muteCursor->m_end = muteCursor->m_timer.getElapsedTime() + m_duration;
	return muteCursor;
}

void MuteGrain::updateCursor(ISoundBufferCursor* cursor) const
{
}

const IGrain* MuteGrain::getCurrentGrain(ISoundBufferCursor* cursor) const
{
	return this;
}

bool MuteGrain::getBlock(const ISoundMixer* mixer, ISoundBufferCursor* cursor, SoundBlock& outBlock) const
{
	MuteGrainCursor* muteCursor = static_cast< MuteGrainCursor* >(cursor);
	return muteCursor->m_timer.getElapsedTime() <= muteCursor->m_end;
}

bool MuteGrain::serialize(ISerializer& s)
{
	return s >> Member< double >(L"duration", m_duration);
}

	}
}
