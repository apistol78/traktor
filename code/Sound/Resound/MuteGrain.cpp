#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
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
	bool m_first;
	double m_timeEnd;
	double m_time;

	virtual void setCursor(double time)
	{
		if (m_first)
		{
			m_timeEnd += time;
			m_first = false;
		}

		m_time = time;
	}
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
	muteCursor->m_first = true;
	muteCursor->m_timeEnd = m_duration;
	muteCursor->m_time = 0.0;
	return muteCursor;
}

void MuteGrain::updateCursor(ISoundBufferCursor* cursor) const
{
}

const IGrain* MuteGrain::getCurrentGrain(ISoundBufferCursor* cursor) const
{
	return this;
}

bool MuteGrain::getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const
{
	MuteGrainCursor* muteCursor = static_cast< MuteGrainCursor* >(cursor);
	return muteCursor->m_time <= muteCursor->m_timeEnd;
}

bool MuteGrain::serialize(ISerializer& s)
{
	return s >> Member< double >(L"duration", m_duration);
}

	}
}
