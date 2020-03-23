#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ITimeQuery.h"
#include "Render/Capture/Error.h"
#include "Render/Capture/IndexBufferCapture.h"
#include "Render/Capture/ProgramCapture.h"
#include "Render/Capture/RenderTargetSetCapture.h"
#include "Render/Capture/RenderViewCapture.h"
#include "Render/Capture/VertexBufferCapture.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewCapture", RenderViewCapture, IRenderView)

RenderViewCapture::RenderViewCapture(IRenderSystem* renderSystem, IRenderView* renderView)
:	m_renderSystem(renderSystem)
,	m_renderView(renderView)
,	m_insideFrame(false)
,	m_insidePass(false)
{
	m_timeQuery = renderSystem->createTimeQuery();
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

bool RenderViewCapture::isMinimized() const
{
	return m_renderView->isMinimized();
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

bool RenderViewCapture::isCursorVisible() const
{
	return m_renderView->isCursorVisible();
}

bool RenderViewCapture::setGamma(float gamma)
{
	return m_renderView->setGamma(gamma);
}

void RenderViewCapture::setViewport(const Viewport& viewport)
{
	m_renderView->setViewport(viewport);
}

SystemWindow RenderViewCapture::getSystemWindow()
{
	return m_renderView->getSystemWindow();
}

bool RenderViewCapture::beginFrame()
{
	T_CAPTURE_ASSERT(!m_insideFrame, L"Frame already begun.");
	if (!m_renderView->beginFrame())
		return false;

	m_insideFrame = true;
	return true;
}

void RenderViewCapture::endFrame()
{
	T_CAPTURE_ASSERT(m_insideFrame, L"Frame not begun.");
	m_renderView->endFrame();
	m_insideFrame = false;
}

void RenderViewCapture::present()
{
	T_CAPTURE_ASSERT (!m_insideFrame, L"Cannot present inside beginFrame/endFrame.");
	m_renderView->present();
}

bool RenderViewCapture::beginPass(const Clear* clear)
{
	T_CAPTURE_ASSERT(!m_insidePass, L"Already inside pass.");
	if (!m_renderView->beginPass(clear))
		return false;

	m_insidePass = true;
	return true;
}

bool RenderViewCapture::beginPass(IRenderTargetSet* renderTargetSet, const Clear* clear)
{
	T_CAPTURE_ASSERT(!m_insidePass, L"Already inside pass.");

	RenderTargetSetCapture* rtsc = mandatory_non_null_type_cast< RenderTargetSetCapture* >(renderTargetSet);
	if (!m_renderView->beginPass(
		rtsc->getRenderTargetSet(),
		clear
	))
		return false;

	m_insidePass = true;
	return true;
}

bool RenderViewCapture::beginPass(IRenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear)
{
	T_CAPTURE_ASSERT(!m_insidePass, L"Already inside pass.");

	RenderTargetSetCapture* rtsc = mandatory_non_null_type_cast< RenderTargetSetCapture* >(renderTargetSet);
	T_CAPTURE_ASSERT (rtsc->haveColorTexture(renderTarget), L"No such render target.");

	if (!m_renderView->beginPass(
		rtsc->getRenderTargetSet(),
		renderTarget,
		clear
	))
		return false;

	m_insidePass = true;
	return true;
}

void RenderViewCapture::endPass()
{
	T_CAPTURE_ASSERT(m_insidePass, L"Not inside pass.");
	m_renderView->endPass();
	m_insidePass = false;
}

void RenderViewCapture::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	T_CAPTURE_ASSERT (m_insidePass, L"Cannot draw outside of beginPass/endPass.");

	ProgramCapture* programCapture = dynamic_type_cast< ProgramCapture* >(program);
	T_CAPTURE_ASSERT (programCapture, L"Incorrect program type.");

	if (!programCapture)
		return;

	T_CAPTURE_ASSERT (programCapture->m_program, L"Trying to draw with destroyed program.");
	T_CAPTURE_ASSERT (vertexBuffer, L"No vertex buffer.");

	if (!vertexBuffer)
		return;

	VertexBufferCapture* vb = checked_type_cast< VertexBufferCapture* >(vertexBuffer);
	IndexBufferCapture* ib = checked_type_cast< IndexBufferCapture* >(indexBuffer);

	// Validate draw call.
	uint32_t vertexCount = 0;
	switch (primitives.type)
	{
	case PtPoints:
		vertexCount = primitives.count;
		break;

	case PtLineStrip:
		T_ASSERT(0);
		break;

	case PtLines:
		vertexCount = primitives.count * 2;
		break;

	case PtTriangleStrip:
		vertexCount = primitives.count + 2;
		break;

	case PtTriangles:
		vertexCount = primitives.count * 3;
		break;
	}

	if (primitives.indexed)
	{
		T_CAPTURE_ASSERT (ib, L"Drawing indexed primitives but no index buffer.");
		if (!ib)
			return;

		uint32_t maxVertexCount = ib->getBufferSize();
		if (ib->getIndexType() == ItUInt16)
			maxVertexCount /= 2;
		else
			maxVertexCount /= 4;

		T_CAPTURE_ASSERT (primitives.offset + vertexCount <= maxVertexCount, L"Trying to draw more primitives than size of index buffer.");
	}
	else
	{
		T_CAPTURE_ASSERT (!ib, L"Drawing non-indexed primitives but index buffer provided.");

		uint32_t maxVertexCount = vb->getBufferSize() / vb->getVertexSize();
		T_CAPTURE_ASSERT (primitives.offset + vertexCount <= maxVertexCount, L"Trying to draw more primitives than size of vertex buffer.");
	}

	programCapture->verify();

	m_renderView->draw(vb->getVertexBuffer(), ib ? ib->getIndexBuffer() : 0, programCapture->m_program, primitives);
}

void RenderViewCapture::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	T_CAPTURE_ASSERT (m_insidePass, L"Cannot draw outside of beginPass/endPass.");

	ProgramCapture* programCapture = dynamic_type_cast< ProgramCapture* >(program);
	T_CAPTURE_ASSERT (programCapture, L"Incorrect program type.");

	if (!programCapture)
		return;

	T_CAPTURE_ASSERT (programCapture->m_program, L"Trying to draw with destroyed program.");
	T_CAPTURE_ASSERT (vertexBuffer, L"No vertex buffer.");

	if (!vertexBuffer)
		return;

	VertexBufferCapture* vb = checked_type_cast< VertexBufferCapture* >(vertexBuffer);
	IndexBufferCapture* ib = checked_type_cast< IndexBufferCapture* >(indexBuffer);

	// Validate draw call.
	uint32_t vertexCount = 0;
	switch (primitives.type)
	{
	case PtPoints:
		vertexCount = primitives.count;
		break;

	case PtLineStrip:
		T_ASSERT(0);
		break;

	case PtLines:
		vertexCount = primitives.count * 2;
		break;

	case PtTriangleStrip:
		vertexCount = primitives.count + 2;
		break;

	case PtTriangles:
		vertexCount = primitives.count * 3;
		break;
	}

	if (primitives.indexed)
	{
		T_CAPTURE_ASSERT (ib, L"Drawing indexed primitives but no index buffer.");
		if (!ib)
			return;

		uint32_t maxVertexCount = ib->getBufferSize();
		if (ib->getIndexType() == ItUInt16)
			maxVertexCount /= 2;
		else
			maxVertexCount /= 4;

		T_CAPTURE_ASSERT (primitives.offset + vertexCount <= maxVertexCount, L"Trying to draw more primitives than size of index buffer.");
	}
	else
	{
		T_CAPTURE_ASSERT (!ib, L"Drawing non-indexed primitives but index buffer provided.");

		uint32_t maxVertexCount = vb->getBufferSize() / vb->getVertexSize();
		T_CAPTURE_ASSERT (primitives.offset + vertexCount <= maxVertexCount, L"Trying to draw more primitives than size of vertex buffer.");
	}

	programCapture->verify();

	m_renderView->draw(vb->getVertexBuffer(), ib ? ib->getIndexBuffer() : 0, programCapture->m_program, primitives, instanceCount);
}

void RenderViewCapture::compute(IProgram* program, const int32_t* workSize)
{
	T_CAPTURE_ASSERT (m_insidePass, L"Cannot compute outside of beginPass/endPass.");
	T_CAPTURE_ASSERT (workSize != nullptr, L"Incorrect argument; workSize null.");

	ProgramCapture* programCapture = dynamic_type_cast< ProgramCapture* >(program);
	T_CAPTURE_ASSERT (programCapture, L"Incorrect program type.");

	if (!programCapture)
		return;

	programCapture->verify();

	m_renderView->compute(programCapture->m_program, workSize);
}

bool RenderViewCapture::copy(ITexture* destinationTexture, const Region& destinationRegion, ITexture* sourceTexture, const Region& sourceRegion)
{
	T_CAPTURE_ASSERT (m_insidePass, L"Cannot copy outside of beginPass/endPass.");
	T_CAPTURE_ASSERT (destinationTexture, L"Invalid destination texture.");
	T_CAPTURE_ASSERT (sourceTexture, L"Invalid destination texture.");

	return m_renderView->copy(destinationTexture, destinationRegion, sourceTexture, sourceRegion);
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
