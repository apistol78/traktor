#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Net/Replication/State/FloatValue.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.net.FloatValue", 0, FloatValue, IValue)

void FloatValue::serialize(ISerializer& s)
{
	s >> Member< float >(L"value", m_value);
}

	}
}
