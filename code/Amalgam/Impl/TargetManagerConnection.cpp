#include "Amalgam/Impl/TargetID.h"
#include "Amalgam/Impl/TargetManagerConnection.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
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

bool TargetManagerConnection::connect(const std::wstring& host, uint16_t port, const Guid& id)
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
		m_socket = 0;
		return false;
	}

	m_socketStream = new net::SocketStream(m_socket, false, true);

    TargetID targetId(id);
	if (!BinarySerializer(m_socketStream).writeObject(&targetId))
	{
		log::error << L"Failed to create target manager connection; unable to send ID" << Endl;
		m_socketStream = 0;
		m_socket = 0;
		return false;
	}

	log::info << L"Connection to target manager established successfully" << Endl;
	return true;
}

bool TargetManagerConnection::connected() const
{
	return m_socket != 0;
}

void TargetManagerConnection::setPerformance(const TargetPerformance& performance)
{
	m_performance = performance;
}

bool TargetManagerConnection::update()
{
	if (!m_socket || !m_socketStream)
		return false;

	// Check for connection terminate.
	if (m_socket->select(true, false, false, 0) > 0)
	{
		if (m_socket->recv() < 0)
		{
			m_socketStream = 0;
			m_socket->close();
			m_socket = 0;
			return false;
		}
	}

	BinarySerializer(m_socketStream).writeObject(&m_performance);
	return true;
}

	}
}
