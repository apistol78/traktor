#include "Spray/Source.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.Source", Source, Serializable)

Source::Source()
:	m_rate(0.0f)
{
}

bool Source::serialize(Serializer& s)
{
	return s >> Member< float >(L"rate", m_rate);
}

	}
}
