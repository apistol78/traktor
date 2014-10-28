#include "Online/Impl/VideoSharing.h"
#include "Online/Provider/IVideoSharingProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.VideoSharing", VideoSharing, IVideoSharing)

bool VideoSharing::beginCapture(int32_t duration)
{
	if (!m_capturing)
		m_capturing = m_provider->beginCapture(duration);

	return m_capturing;
}

void VideoSharing::endCapture()
{
	if (m_capturing)
	{
		m_provider->endCapture();
		m_capturing = false;
	}
}

bool VideoSharing::isCapturing() const
{
	return m_capturing;
}

bool VideoSharing::showShareUI()
{
	return m_provider->showShareUI();
}

VideoSharing::VideoSharing(IVideoSharingProvider* provider)
:	m_provider(provider)
,	m_capturing(false)
{
}

	}
}
