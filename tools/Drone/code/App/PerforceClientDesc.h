#ifndef traktor_drone_PerforceClientDesc_H
#define traktor_drone_PerforceClientDesc_H

#include <Core/Serialization/ISerializable.h>

namespace traktor
{
	namespace drone
	{

class PerforceClientDesc : public ISerializable
{
	T_RTTI_CLASS

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

	virtual void serialize(ISerializer& s);
};

	}
}

#endif	// traktor_drone_PerforceClientDesc_H
