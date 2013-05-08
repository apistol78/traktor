#include "Spray/SourceData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SourceData", SourceData, ISerializable)

SourceData::SourceData()
:	m_constantRate(0.0f)
,	m_velocityRate(0.0f)
{
}

void SourceData::serialize(ISerializer& s)
{
	s >> Member< float >(L"constantRate", m_constantRate);
	s >> Member< float >(L"velocityRate", m_velocityRate);
}

	}
}
