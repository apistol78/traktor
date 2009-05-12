#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberEnum.h>
#include "App/PerforceClientDesc.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.drone.PerforceClientDesc", PerforceClientDesc, Serializable)

bool PerforceClientDesc::serialize(Serializer& s)
{
	const MemberEnum< SecurityLevel >::Key c_SecurityLevel_Keys[] =
	{
		{ L"SlLow", SlLow },
		{ L"SlHigh", SlHigh },
		0
	};

	s >> Member< std::wstring >(L"host", m_host);
	s >> Member< std::wstring >(L"port", m_port);
	s >> Member< std::wstring >(L"user", m_user);
	s >> Member< std::wstring >(L"password", m_password);
	s >> Member< std::wstring >(L"client", m_client);
	s >> MemberEnum< SecurityLevel >(L"securityLevel", m_securityLevel, c_SecurityLevel_Keys);

	return true;
}

	}
}
