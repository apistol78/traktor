#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Sound/IFilter.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/ISoundResource.h"
#include "Sound/Sound.h"
#include "Sound/SoundBlockUtilities.h"
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
	RefArray< IFilterInstance > m_filterInstances;
	double m_timeOffset;
	float m_gain;
	float m_pitch;

	virtual void setCursor(double time)
	{
		if (m_timeOffset < 0.0)
			m_timeOffset = time;

		m_soundCursor->setCursor(time - m_timeOffset);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.PlayGrain", 2, PlayGrain, IGrain)

PlayGrain::PlayGrain()
:	m_gain(0.0f, 0.0f)
,	m_pitch(1.0f, 1.0f)
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
	playCursor->m_gain = m_gain.random(m_random);
	playCursor->m_pitch = m_pitch.random(m_random);

	for (RefArray< IFilter >::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
		playCursor->m_filterInstances.push_back((*i) ? (*i)->createInstance() : 0);

	return playCursor;
}

void PlayGrain::updateCursor(ISoundBufferCursor* cursor) const
{
	PlayGrainCursor* playCursor = static_cast< PlayGrainCursor* >(cursor);
	
	playCursor->m_filterInstances.resize(0);
	for (RefArray< IFilter >::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
		playCursor->m_filterInstances.push_back((*i) ? (*i)->createInstance() : 0);
}

const IGrain* PlayGrain::getCurrentGrain(ISoundBufferCursor* cursor) const
{
	return this;
}

bool PlayGrain::getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const
{
	PlayGrainCursor* playCursor = static_cast< PlayGrainCursor* >(cursor);
	if (!playCursor->m_soundBuffer->getBlock(
		playCursor->m_soundCursor,
		outBlock
	))
		return false;

	uint32_t nfilters = m_filters.size();
	for (uint32_t i = 0; i < nfilters; ++i)
	{
		if (m_filters[i])
			m_filters[i]->apply(playCursor->m_filterInstances[i], outBlock);
	}

	if (abs(playCursor->m_gain) > FUZZY_EPSILON || abs(1.0f - playCursor->m_pitch) > FUZZY_EPSILON)
		soundBlockMulConst(outBlock, playCursor->m_gain + 1.0f);

	return true;
}

bool PlayGrain::serialize(ISerializer& s)
{
	s >> resource::Member< Sound, ISoundResource >(L"sound", m_sound);
	if (s.getVersion() >= 1)
		s >> MemberRefArray< IFilter >(L"filters", m_filters);
	if (s.getVersion() >= 2)
	{
		s >> MemberComposite< Range< float > >(L"gain", m_gain);
		s >> MemberComposite< Range< float > >(L"pitch", m_pitch);
	}
	else
	{
		float gain = 0.0f;
		s >> Member< float >(L"gain", gain);
		m_gain = Range< float >(gain, gain);
	}
	return true;
}

	}
}
