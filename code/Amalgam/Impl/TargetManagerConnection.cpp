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
		return false;
	}

	m_socketStream = new net::SocketStream(m_socket, false, true);

    TargetID targetId(id);
	if (!BinarySerializer(m_socketStream).writeObject(&targetId))
	{
		log::error << L"Failed to create target manager connection; unable to send ID" << Endl;
		return false;
	}

	log::info << L"Connection to target manager established successfully" << Endl;
	return true;
}

void TargetManagerConnection::setPerformance(const TargetPerformance& performance)
{
	const float c_filterCoeff = 0.1f;
	float dT = performance.time - m_performance.time;

	m_deltaPerformance.time = performance.time;
	m_deltaPerformance.fps = lerp(m_deltaPerformance.fps, abs((performance.fps - m_performance.fps) / dT), c_filterCoeff);
	m_deltaPerformance.update = lerp(m_deltaPerformance.update, abs((performance.update - m_performance.update) / dT), c_filterCoeff);
	m_deltaPerformance.build = lerp(m_deltaPerformance.build, abs((performance.build - m_performance.build) / dT), c_filterCoeff);
	m_deltaPerformance.render = lerp(m_deltaPerformance.render, abs((performance.render - m_performance.render) / dT), c_filterCoeff);
	m_deltaPerformance.physics = lerp(m_deltaPerformance.physics, abs((performance.physics - m_performance.physics) / dT), c_filterCoeff);
	m_deltaPerformance.input = lerp(m_deltaPerformance.input, abs((performance.input - m_performance.input) / dT), c_filterCoeff);
	m_deltaPerformance.steps = lerp(m_deltaPerformance.steps, abs((performance.steps - m_performance.steps) / dT), c_filterCoeff);
	m_deltaPerformance.interval = lerp(m_deltaPerformance.interval, abs((performance.interval - m_performance.interval) / dT), c_filterCoeff);
	m_deltaPerformance.memInUse = 0;
	m_deltaPerformance.heapObjects = 0;

	m_performance = performance;
}

void TargetManagerConnection::update()
{
	BinarySerializer(m_socketStream).writeObject(&m_performance);
	BinarySerializer(m_socketStream).writeObject(&m_deltaPerformance);
}

	}
}
