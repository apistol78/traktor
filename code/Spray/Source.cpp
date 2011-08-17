#include "Spray/Source.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.Source", Source, ISerializable)

Source::Source()
:	m_constantRate(0.0f)
,	m_velocityRate(0.0f)
{
}

bool Source::serialize(ISerializer& s)
{
	s >> Member< float >(L"constantRate", m_constantRate);
	s >> Member< float >(L"velocityRate", m_velocityRate);
	return true;
}

	}
}
