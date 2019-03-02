#include "Net/Discovery/DmFindServices.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberType.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

class MemberSocketAddressIPv4 : public MemberComplex
{
public:
	MemberSocketAddressIPv4(const wchar_t* const name, SocketAddressIPv4& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		uint32_t addr = m_ref.getAddr();
		uint16_t port = m_ref.getPort();
		s >> Member< uint32_t >(L"addr", addr);
		s >> Member< uint16_t >(L"port", port);
		m_ref = SocketAddressIPv4(addr, port);
	}

private:
	SocketAddressIPv4& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.net.DmFindServices", 0, DmFindServices, IDiscoveryMessage)

DmFindServices::DmFindServices()
{
}

DmFindServices::DmFindServices(const Guid& managerGuid, const SocketAddressIPv4& replyTo)
:	m_managerGuid(managerGuid)
,	m_replyTo(replyTo)
{
}

void DmFindServices::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"managerGuid", m_managerGuid);
	s >> MemberSocketAddressIPv4(L"replyTo", m_replyTo);
}

	}
}
