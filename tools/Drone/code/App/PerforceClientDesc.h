#ifndef traktor_drone_PerforceClientDesc_H
#define traktor_drone_PerforceClientDesc_H

#include <Core/Serialization/Serializable.h>

namespace traktor
{
	namespace drone
	{

class PerforceClientDesc : public Serializable
{
	T_RTTI_CLASS(PerforceClientDesc)

public:
	enum SecurityLevel
	{
		SlLow,
		SlHigh
	};

	std::wstring m_host;
	std::wstring m_port;
	std::wstring m_user;
	std::wstring m_password;
	std::wstring m_client;
	SecurityLevel m_securityLevel;

	virtual bool serialize(Serializer& s);
};

	}
}

#endif	// traktor_drone_PerforceClientDesc_H
