#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Net/SocketAddressIPv4.h"
#include "Amalgam/Impl/TargetManagerConnection.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetManagerConnection", TargetManagerConnection, Object)

TargetManagerConnection::TargetManagerConnection()
{
}

bool TargetManagerConnection::connect(const std::wstring& host, uint16_t port)
{
	m_socket = new net::TcpSocket();
	if (!m_socket->connect(net::SocketAddressIPv4(host, port)))
	{
		log::error << L"Unable to connect to target manager" << Endl;
		return false;
	}

	m_socketStream = new net::SocketStream(m_socket, false, true);

	log::info << L"Connnection to target manager established successfully" << Endl;
	return true;
}

void TargetManagerConnection::setPerformance(const TargetPerformance& performance)
{
	m_performance = performance;
}

void TargetManagerConnection::update()
{
	BinarySerializer(m_socketStream).writeObject(&m_performance);
}

	}
}
