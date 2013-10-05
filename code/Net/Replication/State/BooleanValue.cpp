#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Net/Replication/State/BooleanValue.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.net.BooleanValue", 0, BooleanValue, IValue)

void BooleanValue::serialize(ISerializer& s)
{
	s >> Member< bool >(L"value", m_value);
}

	}
}
