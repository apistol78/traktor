#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Vrfy/Error.h"
#include "Render/Vrfy/CubeTextureVrfy.h"
#include "Render/Vrfy/IndexBufferVrfy.h"
#include "Render/Vrfy/ProgramVrfy.h"
#include "Render/Vrfy/RenderTargetSetVrfy.h"
#include "Render/Vrfy/RenderViewVrfy.h"
#include "Render/Vrfy/SimpleTextureVrfy.h"
#include "Render/Vrfy/VertexBufferVrfy.h"
#include "Render/Vrfy/VolumeTextureVrfy.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewVrfy", RenderViewVrfy, IRenderView)

RenderViewVrfy::RenderViewVrfy(const RenderViewDesc& desc, IRenderSystem* renderSystem, IRenderView* renderView)
:	m_desc(desc)
,	m_renderSystem(renderSystem)
,	m_renderView(renderView)
,	m_insideFrame(false)
,	m_insidePass(false)
{
}

bool RenderViewVrfy::nextEvent(RenderEvent& outEvent)
{
	return m_renderView->nextEvent(outEvent);
}

void RenderViewVrfy::close()
{
	m_renderView->close();
}

bool RenderViewVrfy::reset(const RenderViewDefaultDesc& desc)
{
	return m_renderView->reset(desc);
}

bool RenderViewVrfy::reset(int32_t width, int32_t height)
{
	return m_renderView->reset(width, height);
}

int RenderViewVrfy::getWidth() const
{
	return m_renderView->getWidth();
}

int RenderViewVrfy::getHeight() const
{
	return m_renderView->getHeight();
}

bool RenderViewVrfy::isActive() const
{
	return m_renderView->isActive();
}

bool RenderViewVrfy::isMinimized() const
{
	return m_renderView->isMinimized();
}

bool RenderViewVrfy::isFullScreen() const
{
	return m_renderView->isFullScreen();
}

void RenderViewVrfy::showCursor()
{
	m_renderView->showCursor();
}

void RenderViewVrfy::hideCursor()
{
	m_renderView->hideCursor();
}

bool RenderViewVrfy::isCursorVisible() const
{
	return m_renderView->isCursorVisible();
}

bool RenderViewVrfy::setGamma(float gamma)
{
	return m_renderView->setGamma(gamma);
}

void RenderViewVrfy::setViewport(const Viewport& viewport)
{
	m_renderView->setViewport(viewport);
}

SystemWindow RenderViewVrfy::getSystemWindow()
{
	return m_renderView->getSystemWindow();
}

bool RenderViewVrfy::beginFrame()
{
	T_CAPTURE_ASSERT(!m_insideFrame, L"Frame already begun.");
	if (!m_renderView->beginFrame())
		return false;

	m_insideFrame = true;
	return true;
}

void RenderViewVrfy::endFrame()
{
	T_CAPTURE_ASSERT(m_insideFrame, L"Frame not begun.");
	m_renderView->endFrame();
	m_insideFrame = false;
}

void RenderViewVrfy::present()
{
	T_CAPTURE_ASSERT(!m_insideFrame, L"Cannot present inside beginFrame/endFrame.");
	m_renderView->present();
}

bool RenderViewVrfy::beginPass(const Clear* clear, uint32_t load, uint32_t store)
{
	T_CAPTURE_ASSERT(!m_insidePass, L"Already inside pass.");
	if (!m_renderView->beginPass(clear, load, store))
		return false;

	m_insidePass = true;
	return true;
}

bool RenderViewVrfy::beginPass(IRenderTargetSet* renderTargetSet, const Clear* clear, uint32_t load, uint32_t store)
{
	T_CAPTURE_ASSERT(!m_insidePass, L"Already inside pass.");

	RenderTargetSetVrfy* rtsc = mandatory_non_null_type_cast< RenderTargetSetVrfy* >(renderTargetSet);

	if (rtsc->usingPrimaryDepthStencil())
	{
		T_CAPTURE_ASSERT(rtsc->getMultiSample() == m_desc.multiSample, L"Trying to render to RenderTargetSet with incompatible multisample configuration.");
	}

	if (!m_renderView->beginPass(
		rtsc->getRenderTargetSet(),
		clear,
		load,
		store
	))
		return false;

	m_insidePass = true;
	return true;
}

bool RenderViewVrfy::beginPass(IRenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear, uint32_t load, uint32_t store)
{
	T_CAPTURE_ASSERT(!m_insidePass, L"Already inside pass.");

	RenderTargetSetVrfy* rtsc = mandatory_non_null_type_cast< RenderTargetSetVrfy* >(renderTargetSet);
	T_CAPTURE_ASSERT(rtsc->haveColorTexture(renderTarget), L"No such render target.");

	if (!m_renderView->beginPass(
		rtsc->getRenderTargetSet(),
		renderTarget,
		clear,
		load,
		store
	))
		return false;

	m_insidePass = true;
	return true;
}

void RenderViewVrfy::endPass()
{
	T_CAPTURE_ASSERT(m_insidePass, L"Not inside pass.");
	m_renderView->endPass();
	m_insidePass = false;
}

void RenderViewVrfy::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	T_CAPTURE_ASSERT(m_insidePass, L"Cannot draw outside of beginPass/endPass.");

	ProgramVrfy* programVrfy = dynamic_type_cast< ProgramVrfy* >(program);
	T_CAPTURE_ASSERT(programVrfy, L"Incorrect program type.");

	if (!programVrfy)
		return;

	T_CAPTURE_ASSERT(programVrfy->m_program, L"Trying to draw with destroyed program.");
	T_CAPTURE_ASSERT(vertexBuffer, L"No vertex buffer.");

	if (!vertexBuffer)
		return;

	VertexBufferVrfy* vb = checked_type_cast< VertexBufferVrfy* >(vertexBuffer);
	IndexBufferVrfy* ib = checked_type_cast< IndexBufferVrfy* >(indexBuffer);

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
		T_CAPTURE_ASSERT(ib, L"Drawing indexed primitives but no index buffer.");
		if (!ib)
			return;

		uint32_t maxVertexCount = ib->getBufferSize();
		if (ib->getIndexType() == ItUInt16)
			maxVertexCount /= 2;
		else
			maxVertexCount /= 4;

		T_CAPTURE_ASSERT(primitives.offset + vertexCount <= maxVertexCount, L"Trying to draw more primitives than size of index buffer.");
	}
	else
	{
		T_CAPTURE_ASSERT(!ib, L"Drawing non-indexed primitives but index buffer provided.");

		uint32_t maxVertexCount = vb->getBufferSize() / vb->getVertexSize();
		T_CAPTURE_ASSERT(primitives.offset + vertexCount <= maxVertexCount, L"Trying to draw more primitives than size of vertex buffer.");
	}

	programVrfy->verify();

	m_renderView->draw(vb->getVertexBuffer(), ib ? ib->getIndexBuffer() : 0, programVrfy->m_program, primitives);
}

void RenderViewVrfy::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	T_CAPTURE_ASSERT(m_insidePass, L"Cannot draw outside of beginPass/endPass.");

	ProgramVrfy* programVrfy = dynamic_type_cast< ProgramVrfy* >(program);
	T_CAPTURE_ASSERT(programVrfy, L"Incorrect program type.");

	if (!programVrfy)
		return;

	T_CAPTURE_ASSERT(programVrfy->m_program, L"Trying to draw with destroyed program.");
	T_CAPTURE_ASSERT(vertexBuffer, L"No vertex buffer.");

	if (!vertexBuffer)
		return;

	VertexBufferVrfy* vb = checked_type_cast< VertexBufferVrfy* >(vertexBuffer);
	IndexBufferVrfy* ib = checked_type_cast< IndexBufferVrfy* >(indexBuffer);

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
		T_CAPTURE_ASSERT(ib, L"Drawing indexed primitives but no index buffer.");
		if (!ib)
			return;

		uint32_t maxVertexCount = ib->getBufferSize();
		if (ib->getIndexType() == ItUInt16)
			maxVertexCount /= 2;
		else
			maxVertexCount /= 4;

		T_CAPTURE_ASSERT(primitives.offset + vertexCount <= maxVertexCount, L"Trying to draw more primitives than size of index buffer.");
	}
	else
	{
		T_CAPTURE_ASSERT(!ib, L"Drawing non-indexed primitives but index buffer provided.");

		uint32_t maxVertexCount = vb->getBufferSize() / vb->getVertexSize();
		T_CAPTURE_ASSERT(primitives.offset + vertexCount <= maxVertexCount, L"Trying to draw more primitives than size of vertex buffer.");
	}

	programVrfy->verify();

	m_renderView->draw(vb->getVertexBuffer(), ib ? ib->getIndexBuffer() : 0, programVrfy->m_program, primitives, instanceCount);
}

void RenderViewVrfy::compute(IProgram* program, const int32_t* workSize)
{
	T_CAPTURE_ASSERT(m_insidePass, L"Cannot compute outside of beginPass/endPass.");
	T_CAPTURE_ASSERT(workSize != nullptr, L"Incorrect argument; workSize null.");

	ProgramVrfy* programVrfy = dynamic_type_cast< ProgramVrfy* >(program);
	T_CAPTURE_ASSERT(programVrfy, L"Incorrect program type.");

	if (!programVrfy)
		return;

	programVrfy->verify();

	m_renderView->compute(programVrfy->m_program, workSize);
}

bool RenderViewVrfy::copy(ITexture* destinationTexture, const Region& destinationRegion, ITexture* sourceTexture, const Region& sourceRegion)
{
	T_CAPTURE_ASSERT(destinationTexture, L"Invalid destination texture.");
	T_CAPTURE_ASSERT(sourceTexture, L"Invalid destination texture.");
	T_CAPTURE_ASSERT(!m_insidePass, L"Cannot copy while being in an active render pass.");

	ITexture* destinationTextureUnwrapped = nullptr;
	ITexture* sourceTextureUnwrapped = nullptr;

	if (CubeTextureVrfy* cubeTexture = dynamic_type_cast< CubeTextureVrfy* >(destinationTexture))
	{
		T_CAPTURE_ASSERT(destinationRegion.mip >= 0, L"Invalid arguments.");
		T_CAPTURE_ASSERT(destinationRegion.mip < cubeTexture->getMips(), L"Invalid arguments.");

		int32_t side = cubeTexture->getSide() >> destinationRegion.mip;

		T_CAPTURE_ASSERT(destinationRegion.x >= 0, L"Invalid arguments.");
		T_CAPTURE_ASSERT(destinationRegion.x + destinationRegion.width <= side, L"Invalid arguments.");
		T_CAPTURE_ASSERT(destinationRegion.y >= 0, L"Invalid arguments.");
		T_CAPTURE_ASSERT(destinationRegion.y + destinationRegion.height <= side, L"Invalid arguments.");
		T_CAPTURE_ASSERT(destinationRegion.z >= 0, L"Invalid arguments.");
		T_CAPTURE_ASSERT(destinationRegion.z < 6, L"Invalid arguments.");

		destinationTextureUnwrapped = cubeTexture->getTexture();
	}
	else if (SimpleTextureVrfy* simpleTexture = dynamic_type_cast< SimpleTextureVrfy* >(destinationTexture))
	{
		T_CAPTURE_ASSERT(destinationRegion.mip >= 0, L"Invalid arguments.");
		T_CAPTURE_ASSERT(destinationRegion.mip < simpleTexture->getMips(), L"Invalid arguments.");

		int32_t width = simpleTexture->getWidth() >> destinationRegion.mip;
		int32_t height = simpleTexture->getHeight() >> destinationRegion.mip;

		T_CAPTURE_ASSERT(destinationRegion.x >= 0, L"Invalid arguments.");
		T_CAPTURE_ASSERT(destinationRegion.x + destinationRegion.width <= width, L"Invalid arguments.");
		T_CAPTURE_ASSERT(destinationRegion.y >= 0, L"Invalid arguments.");
		T_CAPTURE_ASSERT(destinationRegion.y + destinationRegion.height <= height, L"Invalid arguments.");
		T_CAPTURE_ASSERT(destinationRegion.z == 0, L"Invalid arguments.");

		destinationTextureUnwrapped = simpleTexture->getTexture();
	}
	else if (VolumeTextureVrfy* volumeTexture = dynamic_type_cast< VolumeTextureVrfy* >(destinationTexture))
		destinationTextureUnwrapped = volumeTexture->getTexture();
	else
		T_FATAL_ERROR;

	if (CubeTextureVrfy* cubeTexture = dynamic_type_cast< CubeTextureVrfy* >(sourceTexture))
	{
		T_CAPTURE_ASSERT(sourceRegion.mip >= 0, L"Invalid arguments.");
		T_CAPTURE_ASSERT(sourceRegion.mip < cubeTexture->getMips(), L"Invalid arguments.");

		int32_t side = cubeTexture->getSide() >> sourceRegion.mip;

		T_CAPTURE_ASSERT(sourceRegion.x >= 0, L"Invalid arguments.");
		T_CAPTURE_ASSERT(sourceRegion.x + sourceRegion.width <= side, L"Invalid arguments.");
		T_CAPTURE_ASSERT(sourceRegion.y >= 0, L"Invalid arguments.");
		T_CAPTURE_ASSERT(sourceRegion.y + sourceRegion.height <= side, L"Invalid arguments.");
		T_CAPTURE_ASSERT(sourceRegion.z >= 0, L"Invalid arguments.");
		T_CAPTURE_ASSERT(sourceRegion.z < 6, L"Invalid arguments.");

		sourceTextureUnwrapped = cubeTexture->getTexture();
	}
	else if (SimpleTextureVrfy* simpleTexture = dynamic_type_cast< SimpleTextureVrfy* >(sourceTexture))
	{
		T_CAPTURE_ASSERT(sourceRegion.mip >= 0, L"Invalid arguments.");
		T_CAPTURE_ASSERT(sourceRegion.mip < simpleTexture->getMips(), L"Invalid arguments.");

		int32_t width = simpleTexture->getWidth() >> sourceRegion.mip;
		int32_t height = simpleTexture->getHeight() >> sourceRegion.mip;

		T_CAPTURE_ASSERT(sourceRegion.x >= 0, L"Invalid arguments.");
		T_CAPTURE_ASSERT(sourceRegion.x + sourceRegion.width <= width, L"Invalid arguments.");
		T_CAPTURE_ASSERT(sourceRegion.y >= 0, L"Invalid arguments.");
		T_CAPTURE_ASSERT(sourceRegion.y + sourceRegion.height <= height, L"Invalid arguments.");
		T_CAPTURE_ASSERT(sourceRegion.z == 0, L"Invalid arguments.");

		sourceTextureUnwrapped = simpleTexture->getTexture();
	}
	else if (VolumeTextureVrfy* volumeTexture = dynamic_type_cast< VolumeTextureVrfy* >(sourceTexture))
		sourceTextureUnwrapped = volumeTexture->getTexture();
	else
		T_FATAL_ERROR;

	T_CAPTURE_ASSERT(destinationTextureUnwrapped, L"Using destroyed texture as destination.");
	T_CAPTURE_ASSERT(sourceTextureUnwrapped, L"Using destroyed texture as source.");
	if (destinationTextureUnwrapped == nullptr || sourceTextureUnwrapped == nullptr)
		return false;

	return m_renderView->copy(destinationTextureUnwrapped, destinationRegion, sourceTextureUnwrapped, sourceRegion);
}

int32_t RenderViewVrfy::beginTimeQuery()
{
	return m_renderView->beginTimeQuery();
}

void RenderViewVrfy::endTimeQuery(int32_t query)
{
	m_renderView->endTimeQuery(query);
}

bool RenderViewVrfy::getTimeQuery(int32_t query, bool wait, double& outStart, double& outEnd) const
{
	return m_renderView->getTimeQuery(query, wait, outStart, outEnd);
}

void RenderViewVrfy::pushMarker(const char* const marker)
{
	m_renderView->pushMarker(marker);
}

void RenderViewVrfy::popMarker()
{
	m_renderView->popMarker();
}

void RenderViewVrfy::getStatistics(RenderViewStatistics& outStatistics) const
{
	m_renderView->getStatistics(outStatistics);
}

	}
}
