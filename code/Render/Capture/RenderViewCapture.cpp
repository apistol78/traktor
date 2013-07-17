#if defined(_WIN32)
#	include <windows.h>
#endif
#include "Render/Capture/RenderViewCapture.h"
#include "Render/IRenderSystem.h"
#include "Render/RenderTargetSet.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewCapture", RenderViewCapture, IRenderView)

RenderViewCapture::RenderViewCapture(IRenderSystem* renderSystem, IRenderView* renderView)
:	m_renderSystem(renderSystem)
,	m_renderView(renderView)
,	m_captureFrame(false)
,	m_captureDepth(0)
,	m_drawCount(0)
,	m_frameCount(0)
{
}

bool RenderViewCapture::nextEvent(RenderEvent& outEvent)
{
	return m_renderView->nextEvent(outEvent);
}

void RenderViewCapture::close()
{
	m_renderView->close();
}

bool RenderViewCapture::reset(const RenderViewDefaultDesc& desc)
{
	return m_renderView->reset(desc);
}

bool RenderViewCapture::reset(int32_t width, int32_t height)
{
	return m_renderView->reset(width, height);
}

int RenderViewCapture::getWidth() const
{
	return m_renderView->getWidth();
}

int RenderViewCapture::getHeight() const
{
	return m_renderView->getHeight();
}

bool RenderViewCapture::isActive() const
{
	return m_renderView->isActive();
}

bool RenderViewCapture::isFullScreen() const
{
	return m_renderView->isFullScreen();
}

void RenderViewCapture::showCursor()
{
	m_renderView->showCursor();
}

void RenderViewCapture::hideCursor()
{
	m_renderView->hideCursor();
}

bool RenderViewCapture::setGamma(float gamma)
{
	return m_renderView->setGamma(gamma);
}

void RenderViewCapture::setViewport(const Viewport& viewport)
{
	m_renderView->setViewport(viewport);
}

Viewport RenderViewCapture::getViewport()
{
	return m_renderView->getViewport();
}

SystemWindow RenderViewCapture::getSystemWindow()
{
	return m_renderView->getSystemWindow();
}

bool RenderViewCapture::begin(EyeType eye)
{
	if (!m_renderView->begin(eye))
		return false;

#if defined(_DEBUG)
	static bool s_captureFrame = false;
	if (s_captureFrame)
	{
		m_captureFrame = true;
		s_captureFrame = false;
	}
#endif

#if defined(_WIN32)
	if (GetAsyncKeyState(VK_F12) & 0x8000)
	{
		log::info << L"Render capture begun" << Endl;
		m_captureFrame = true;
	}
#endif

	if (m_captureFrame)
	{
		// Create off screen render target matching dimensions of this view.
		Viewport vp = m_renderView->getViewport();

		RenderTargetSetCreateDesc rtscd;
		rtscd.count = 1;
		rtscd.width = vp.width;
		rtscd.height = vp.height;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = true;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.targets[0].format = TfR8G8B8A8;

		m_captureTarget = m_renderSystem->createRenderTargetSet(rtscd);
		if (!m_captureTarget)
		{
			m_captureFrame = false;
			return m_renderView->begin(EtCyclop);
		}

		if (!m_renderView->begin(m_captureTarget, 0))
		{
			m_captureFrame = false;
			m_captureTarget->destroy();
			m_captureTarget = 0;
			return m_renderView->begin(EtCyclop);
		}

		m_captureImage = new drawing::Image(
			drawing::PixelFormat::getR8G8B8A8(),
			vp.width,
			vp.height
		);
	}

	m_captureDepth = 0;
	return true;
}

bool RenderViewCapture::begin(RenderTargetSet* renderTargetSet)
{
	if (!m_renderView->begin(renderTargetSet))
		return false;

	m_captureDepth++;
	return true;
}

bool RenderViewCapture::begin(RenderTargetSet* renderTargetSet, int renderTarget)
{
	if (!m_renderView->begin(renderTargetSet, renderTarget))
		return false;

	m_captureDepth++;
	return true;
}

void RenderViewCapture::clear(uint32_t clearMask, const Color4f* color, float depth, int32_t stencil)
{
	m_renderView->clear(clearMask, color, depth, stencil);
}

void RenderViewCapture::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	m_renderView->draw(vertexBuffer, indexBuffer, program, primitives);
	m_drawCount++;

	if (m_captureFrame && m_captureDepth == 0)
	{
		m_renderView->end();

		if (m_captureTarget->read(0, m_captureImage->getData()))
		{
			FileSystem::getInstance().makeDirectory(L"capture");

			StringOutputStream ss;
			ss << L"capture/frame" << m_frameCount << L"_draw" << m_drawCount << L".tga";

			if (m_captureImage->save(ss.str()))
				log::info << L"Captured draw " << m_drawCount << L", frame " << m_frameCount << Endl;
			else
				log::error << L"Unable to save captured image \"" << ss.str() << L"\"" << Endl;
		}
		else
			log::error << L"Unable to capture image" << Endl;

		if (!m_renderView->begin(m_captureTarget, 0))
			log::error << L"Unable to continue capture; failed to rebind capture target" << Endl;
	}
}

void RenderViewCapture::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	m_renderView->draw(vertexBuffer, indexBuffer, program, primitives, instanceCount);
	m_drawCount++;

	if (m_captureFrame && m_captureDepth == 0)
	{
		m_renderView->end();

		if (m_captureTarget->read(0, m_captureImage->getData()))
		{
			FileSystem::getInstance().makeDirectory(L"capture");

			StringOutputStream ss;
			ss << L"capture/frame" << m_frameCount << L"_draw" << m_drawCount << L".tga";

			if (m_captureImage->save(ss.str()))
				log::info << L"Captured draw " << m_drawCount << L", frame " << m_frameCount << Endl;
			else
				log::error << L"Unable to save captured image \"" << ss.str() << L"\"" << Endl;
		}
		else
			log::error << L"Unable to capture image" << Endl;

		if (!m_renderView->begin(m_captureTarget, 0))
			log::error << L"Unable to continue capture; failed to rebind capture target" << Endl;
	}
}

void RenderViewCapture::end()
{
	if (m_captureFrame && m_captureDepth == 0)
	{
		m_renderView->end();

		m_captureFrame = false;
		m_captureTarget->destroy();
		m_captureTarget = 0;

		log::info << L"Render capture finished" << Endl;
	}

	m_renderView->end();
	m_captureDepth--;
}

void RenderViewCapture::present()
{
	m_renderView->present();
	m_frameCount++;
}

void RenderViewCapture::pushMarker(const char* const marker)
{
	m_renderView->pushMarker(marker);
}

void RenderViewCapture::popMarker()
{
	m_renderView->popMarker();
}

void RenderViewCapture::getStatistics(RenderViewStatistics& outStatistics) const
{
	m_renderView->getStatistics(outStatistics);
}

bool RenderViewCapture::getBackBufferContent(void* buffer) const
{
	return m_renderView->getBackBufferContent(buffer);
}

	}
}
