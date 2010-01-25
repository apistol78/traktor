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

void RenderViewCapture::close()
{
	m_renderView->close();
}

void RenderViewCapture::resize(int32_t width, int32_t height)
{
	m_renderView->resize(width, height);
}

void RenderViewCapture::setViewport(const Viewport& viewport)
{
	m_renderView->setViewport(viewport);
}

Viewport RenderViewCapture::getViewport()
{
	return m_renderView->getViewport();
}

bool RenderViewCapture::getNativeAspectRatio(float& outAspectRatio) const
{
	return m_renderView->getNativeAspectRatio(outAspectRatio);
}

bool RenderViewCapture::begin()
{
	if (!m_renderView->begin())
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
		rtscd.depthStencil = true;
		rtscd.targets[0].format = TfR8G8B8A8;

		m_captureTarget = m_renderSystem->createRenderTargetSet(rtscd);
		if (!m_captureTarget)
		{
			m_captureFrame = false;
			return m_renderView->begin();
		}

		if (!m_renderView->begin(m_captureTarget, 0, false))
		{
			m_captureFrame = false;
			m_captureTarget->destroy();
			m_captureTarget = 0;
			return m_renderView->begin();
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

bool RenderViewCapture::begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil)
{
	if (!m_renderView->begin(renderTargetSet, renderTarget, keepDepthStencil))
		return false;

	m_captureDepth++;
	return true;
}

void RenderViewCapture::clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil)
{
	m_renderView->clear(clearMask, color, depth, stencil);
}

void RenderViewCapture::setVertexBuffer(VertexBuffer* vertexBuffer)
{
	m_renderView->setVertexBuffer(vertexBuffer);
}

void RenderViewCapture::setIndexBuffer(IndexBuffer* indexBuffer)
{
	m_renderView->setIndexBuffer(indexBuffer);
}

void RenderViewCapture::setProgram(IProgram* program)
{
	m_renderView->setProgram(program);
}

void RenderViewCapture::draw(const Primitives& primitives)
{
	m_renderView->draw(primitives);
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

		if (!m_renderView->begin(m_captureTarget, 0, false))
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

void RenderViewCapture::setMSAAEnable(bool msaaEnable)
{
	m_renderView->setMSAAEnable(msaaEnable);
}

	}
}
