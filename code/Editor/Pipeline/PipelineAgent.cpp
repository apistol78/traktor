#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Editor/IPipeline.h"
#include "Editor/PipelineDependency.h"
#include "Editor/Pipeline/AgentBuild.h"
#include "Editor/Pipeline/AgentConnect.h"
#include "Editor/Pipeline/AgentOpenFile.h"
#include "Editor/Pipeline/AgentStatus.h"
#include "Editor/Pipeline/AgentStream.h"
#include "Editor/Pipeline/PipelineAgent.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/TcpSocket.h"
#include "Net/Stream/StreamServer.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineAgent", PipelineAgent, Object)

Ref< PipelineAgent > PipelineAgent::create(
	net::StreamServer* streamServer,
	const Guid& sessionId,
	const PropertyGroup* settings,
	const std::wstring& description,
	const std::wstring& host,
	uint16_t port,
	uint16_t databasePort
)
{
	net::SocketAddressIPv4::Interface itf;
	if (!net::SocketAddressIPv4::getBestInterface(itf))
		return nullptr;

	Ref< net::TcpSocket > socket = new net::TcpSocket();
	if (!socket->connect(net::SocketAddressIPv4(host, port)))
		return nullptr;

	Ref< net::BidirectionalObjectTransport > transport = new net::BidirectionalObjectTransport(socket);

	AgentConnect agentConnect(
		sessionId,
		settings,
		itf.addr->getHostName(),
		databasePort,
		streamServer->getListenPort()
	);
	if (!transport->send(&agentConnect))
		return nullptr;

	return new PipelineAgent(streamServer, transport, description);
}

void PipelineAgent::update()
{
	Ref< AgentOpenFile > agentOpenFile;
	if (m_transport->recv< AgentOpenFile >(0, agentOpenFile) >= 1)
	{
		Path filePath = FileSystem::getInstance().getAbsolutePath(agentOpenFile->getFilePath());

		uint32_t publicId;

		Ref< IStream > fileStream = FileSystem::getInstance().open(filePath, File::FmRead);
		if (fileStream)
			publicId = m_streamServer->publish(fileStream);

		AgentStream agentStream(publicId);
		m_transport->send(&agentStream);
	}

	Ref< AgentStatus > agentStatus;
	if (m_transport->recv< AgentStatus >(0, agentStatus) >= 1)
	{
		m_result(agentStatus->getResult());
		m_buildingCount--;
	}
}

bool PipelineAgent::isConnected() const
{
	return m_transport->connected();
}

bool PipelineAgent::isIdle() const
{
	return m_buildingCount <= 0;
}

bool PipelineAgent::build(const PipelineDependency* dependency, const std::function< void(bool) >& result)
{
	AgentBuild agentBuild(
		dependency->pipelineType->getName(),
		dependency->sourceInstanceGuid,
		dependency->sourceAsset,
		dependency->sourceAssetHash,
		dependency->outputPath,
		dependency->outputGuid,
		0/*dependency->reason*/
	);
	if (!m_transport->send(&agentBuild))
	{
		result(false);
		return false;
	}
	m_result = result;
	m_buildingCount++;
	return true;
}

PipelineAgent::PipelineAgent(
	net::StreamServer* streamServer,
	net::BidirectionalObjectTransport* transport,
	const std::wstring& description
)
:	m_streamServer(streamServer)
,	m_transport(transport)
,	m_description(description)
,	m_buildingCount(0)
{
}

	}
}
