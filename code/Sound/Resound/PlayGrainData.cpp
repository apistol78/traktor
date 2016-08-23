#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Resource/Proxy.h"
#include "Sound/IFilter.h"
#include "Sound/Sound.h"
#include "Sound/Resound/IGrainFactory.h"
#include "Sound/Resound/PlayGrain.h"
#include "Sound/Resound/PlayGrainData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.PlayGrainData", 2, PlayGrainData, IGrainData)

PlayGrainData::PlayGrainData()
:	m_gain(0.0f, 0.0f)
,	m_pitch(1.0f, 1.0f)
,	m_repeat(false)
{
}

Ref< IGrain > PlayGrainData::createInstance(IGrainFactory* grainFactory) const
{
	resource::Proxy< Sound > sound;
	if (!grainFactory->getResourceManager()->bind(m_sound, sound))
		return 0;

	return new PlayGrain(
		sound,
		m_filters,
		m_gain,
		m_pitch,
		m_repeat
	);
}

void PlayGrainData::serialize(ISerializer& s)
{
	s >> resource::Member< Sound >(L"sound", m_sound);
	s >> MemberRefArray< IFilter >(L"filters", m_filters);
	
	if (s.getVersion() >= 2)
		s >> MemberComposite< Range< float > >(L"gain", m_gain);
	else
	{
		s >> MemberComposite< Range< float > >(L"gain", m_gain);
		m_gain.min = linearToDecibel(1.0f + m_gain.min);
		m_gain.max = linearToDecibel(1.0f + m_gain.max);
	}

	s >> MemberComposite< Range< float > >(L"pitch", m_pitch);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"repeat", m_repeat);
}

	}
}
