#include "Amalgam\Impl\TargetID.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.TargetID", 0, TargetID, ISerializable)

TargetID::TargetID()
{
}

TargetID::TargetID(const Guid& id)
:	m_id(id)
{
}

const Guid& TargetID::getId() const
{
	return m_id;
}

bool TargetID::serialize(ISerializer& s)
{
	return s >> Member< Guid >(L"id", m_id);
}

	}
}
