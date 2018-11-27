#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Tracker/VolumeEvent.h"
#include "Sound/Tracker/VolumeEventData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.VolumeEventData", 0, VolumeEventData, IEventData)

VolumeEventData::VolumeEventData()
:	m_volume(0.0f)
{
}

VolumeEventData::VolumeEventData(float volume)
:	m_volume(volume)
{
}

Ref< IEvent > VolumeEventData::createInstance() const
{
	return new VolumeEvent(m_volume);
}

void VolumeEventData::serialize(ISerializer& s)
{
	s >> Member< float >(L"volume", m_volume);
}

	}
}