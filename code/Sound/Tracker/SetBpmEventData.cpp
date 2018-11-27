#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Tracker/SetBpmEvent.h"
#include "Sound/Tracker/SetBpmEventData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SetBpmEventData", 0, SetBpmEventData, IEventData)

SetBpmEventData::SetBpmEventData()
:	m_bpm(0)
{
}

SetBpmEventData::SetBpmEventData(int32_t bpm)
:	m_bpm(bpm)
{
}

Ref< IEvent > SetBpmEventData::createInstance() const
{
	return new SetBpmEvent(m_bpm);
}

void SetBpmEventData::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"bpm", m_bpm);
}

	}
}