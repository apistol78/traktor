#include "Amalgam/CaptureScreenShot.h"
#include "Amalgam/CapturedScreenShot.h"
#include "Amalgam/Game/Impl/RenderServer.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Atomic.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/TcpSocket.h"
#include "Render/IRenderView.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.RenderServer", RenderServer, IRenderServer)

RenderServer::RenderServer(net::BidirectionalObjectTransport* transport)
:	m_transport(transport)
,	m_frameRate(0)
{
}

RenderServer::UpdateResult RenderServer::update(PropertyGroup* settings)
{
	if (m_transport)
	{
		Ref< CaptureScreenShot > captureScreenShot;
		if (m_transport->recv< CaptureScreenShot >(0, captureScreenShot) == net::BidirectionalObjectTransport::RtSuccess)
		{
			int32_t width = m_renderView->getWidth();
			int32_t height = m_renderView->getHeight();

			Ref< CapturedScreenShot > capturedScreenShot = new CapturedScreenShot(width, height);

			m_renderView->getBackBufferContent(capturedScreenShot->getData().ptr());

			m_transport->send(capturedScreenShot);
		}
	}
	return UrSuccess;
}

int32_t RenderServer::getFrameRate() const
{
	return m_frameRate;
}

void RenderServer::setFrameRate(int32_t frameRate)
{
	Atomic::exchange(m_frameRate, frameRate);
}

int32_t RenderServer::getThreadFrameQueueCount() const
{
	return 2;
}

	}
}
