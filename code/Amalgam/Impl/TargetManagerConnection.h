#ifndef traktor_amalgam_TargetManagerConnection_H
#define traktor_amalgam_TargetManagerConnection_H

#include "Core/Object.h"

namespace traktor
{
	namespace net
	{

class BidirectionalObjectTransport;

	}

	namespace amalgam
	{

class TargetManagerConnection : public Object
{
	T_RTTI_CLASS;

public:
	bool connect(const std::wstring& host, uint16_t port, const Guid& id);

	bool connected() const;

	bool update();

	net::BidirectionalObjectTransport* getTransport() const { return m_transport; }

private:
	Ref< net::BidirectionalObjectTransport > m_transport;
};

	}
}

#endif	// traktor_amalgam_TargetManagerConnection_H
