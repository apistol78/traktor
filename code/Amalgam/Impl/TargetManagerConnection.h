#ifndef traktor_amalgam_TargetManagerConnection_H
#define traktor_amalgam_TargetManagerConnection_H

#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"
#include "Amalgam/Impl/TargetPerformance.h"

namespace traktor
{
	namespace amalgam
	{

class TargetManagerConnection : public Object
{
	T_RTTI_CLASS;

public:
	TargetManagerConnection();

	bool connect(const std::wstring& host, uint16_t port, const Guid& id);

	bool connected() const;

	void setPerformance(const TargetPerformance& performance);

	bool update();

private:
	Ref< net::TcpSocket > m_socket;
	Ref< net::SocketStream > m_socketStream;
	TargetPerformance m_performance;
	TargetPerformance m_deltaPerformance;
};

	}
}

#endif	// traktor_amalgam_TargetManagerConnection_H
