#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAabb.h"
#include "Spark/CharacterInstance.h"
#include "Spark/SwfMembers.h"
#include "Spark/Debug/InstanceDebugInfo.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.InstanceDebugInfo", InstanceDebugInfo, ISerializable)

InstanceDebugInfo::InstanceDebugInfo()
:	m_visible(false)
{
}

void InstanceDebugInfo::serialize(ISerializer& s)
{
	s >> Member< std::string >(L"name", m_name);
	s >> MemberAabb2(L"bounds", m_bounds);
	s >> Member< Matrix33 >(L"localTransform", m_localTransform);
	s >> Member< Matrix33 >(L"globalTransform", m_globalTransform);
	s >> MemberColorTransform(L"cxform", m_cxform);
	s >> Member< bool >(L"visible", m_visible);
}

	}
}
