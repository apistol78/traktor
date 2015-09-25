#include "Amalgam/Game/Impl/RenderServer.h"
#include "Core/Thread/Atomic.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.RenderServer", RenderServer, IRenderServer)

RenderServer::RenderServer()
:	m_frameRate(0)
{
}

RenderServer::UpdateResult RenderServer::update(PropertyGroup* settings)
{
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
