#include "Amalgam/Impl/TargetManagerConnection.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Net/Network.h"
#include "Net/SocketAddressIPv4.h"

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
	if (!net::Network::initialize())
	{
		log::error << L"Failed to create target manager connection; network initialization failed" << Endl;
		return false;
	}

	m_socket = new net::TcpSocket();
	if (!m_socket->connect(net::SocketAddressIPv4(host, port)))
	{
		log::error << L"Failed to create target manager connection; unable to connect" << Endl;
		return false;
	}

	m_socketStream = new net::SocketStream(m_socket, false, true);

	log::info << L"Connection to target manager established successfully" << Endl;
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
