#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Tracker/VolumeSlideEvent.h"
#include "Sound/Tracker/VolumeSlideEventData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.VolumeSlideEventData", 0, VolumeSlideEventData, IEventData)

VolumeSlideEventData::VolumeSlideEventData()
:	m_amount(0.0f)
{
}

VolumeSlideEventData::VolumeSlideEventData(float amount)
:	m_amount(amount)
{
}

Ref< IEvent > VolumeSlideEventData::createInstance() const
{
	return new VolumeSlideEvent(m_amount);
}

void VolumeSlideEventData::serialize(ISerializer& s)
{
	s >> Member< float >(L"amount", m_amount);
}

	}
}
