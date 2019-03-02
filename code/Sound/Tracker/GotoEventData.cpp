#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Tracker/GotoEvent.h"
#include "Sound/Tracker/GotoEventData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.GotoEventData", 0, GotoEventData, IEventData)

GotoEventData::GotoEventData()
:	m_pattern(0)
,	m_row(0)
{
}

GotoEventData::GotoEventData(int32_t pattern, int32_t row)
:	m_pattern(pattern)
,	m_row(row)
{
}

Ref< IEvent > GotoEventData::createInstance() const
{
	return new GotoEvent(m_pattern, m_row);
}

void GotoEventData::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"pattern", m_pattern);
	s >> Member< int32_t >(L"row", m_row);
}

	}
}
