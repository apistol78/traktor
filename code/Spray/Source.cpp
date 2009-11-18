#include "Spray/Source.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.Source", Source, ISerializable)

Source::Source()
:	m_rate(0.0f)
{
}

bool Source::serialize(ISerializer& s)
{
	return s >> Member< float >(L"rate", m_rate);
}

	}
}
