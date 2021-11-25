#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Thread/ThreadManager.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Vrfy/Error.h"
#include "Render/Vrfy/BufferVrfy.h"
#include "Render/Vrfy/CubeTextureVrfy.h"
#include "Render/Vrfy/ProgramVrfy.h"
#include "Render/Vrfy/RenderTargetSetVrfy.h"
#include "Render/Vrfy/RenderViewVrfy.h"
#include "Render/Vrfy/SimpleTextureVrfy.h"
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
{
}

bool RenderViewVrfy::nextEvent(RenderEvent& outEvent)
{
	T_CAPTURE_TRACE(L"nextEvent");
	return m_renderView->nextEvent(outEvent);
}

void RenderViewVrfy::close()
{
	T_CAPTURE_TRACE(L"close");
	m_renderView->close();
}

bool RenderViewVrfy::reset(const RenderViewDefaultDesc& desc)
{
	T_CAPTURE_TRACE(L"reset");
	T_CAPTURE_ASSERT(!m_insideFrame, L"Cannot reset while rendering frame.");
	return m_renderView->reset(desc);
}

bool RenderViewVrfy::reset(int32_t width, int32_t height)
{
	T_CAPTURE_TRACE(L"reset");
	T_CAPTURE_ASSERT(!m_insideFrame, L"Cannot reset while rendering frame.");
	return m_renderView->reset(width, height);
}

int RenderViewVrfy::getWidth() const
{
	T_CAPTURE_TRACE(L"getWidth");
	return m_renderView->getWidth();
}

int RenderViewVrfy::getHeight() const
{
	T_CAPTURE_TRACE(L"getHeight");
	return m_renderView->getHeight();
}

bool RenderViewVrfy::isActive() const
{
	T_CAPTURE_TRACE(L"isActive");
	return m_renderView->isActive();
}

bool RenderViewVrfy::isMinimized() const
{
	T_CAPTURE_TRACE(L"isMinimized");
	return m_renderView->isMinimized();
}

bool RenderViewVrfy::isFullScreen() const
{
	T_CAPTURE_TRACE(L"isFullScreen");
	return m_renderView->isFullScreen();
}

void RenderViewVrfy::showCursor()
{
	T_CAPTURE_TRACE(L"showCursor");
	m_renderView->showCursor();
}

void RenderViewVrfy::hideCursor()
{
	T_CAPTURE_TRACE(L"hideCursor");
	m_renderView->hideCursor();
}

bool RenderViewVrfy::isCursorVisible() const
{
	T_CAPTURE_TRACE(L"isCursorVisible");
	return m_renderView->isCursorVisible();
}

bool RenderViewVrfy::setGamma(float gamma)
{
	T_CAPTURE_TRACE(L"setGamma");
	return m_renderView->setGamma(gamma);
}

void RenderViewVrfy::setViewport(const Viewport& viewport)
{
	T_CAPTURE_TRACE(L"setViewport");
	T_CAPTURE_ASSERT(m_insideFrame, L"Cannot set viewport outside of frame.");
	m_renderView->setViewport(viewport);
}

SystemWindow RenderViewVrfy::getSystemWindow()
{
	T_CAPTURE_TRACE(L"getSystemWindow");
	return m_renderView->getSystemWindow();
}

bool RenderViewVrfy::beginFrame()
{
	T_CAPTURE_TRACE(L"beginFrame");
	T_CAPTURE_ASSERT(!m_insideFrame, L"Frame already begun.");

	if (!m_renderView->beginFrame())
		return false;

	m_threadFrame = ThreadManager::getInstance().getCurrentThread();
	m_insideFrame = true;
	return true;
}

void RenderViewVrfy::endFrame()
{
	T_CAPTURE_TRACE(L"endFrame");
	T_CAPTURE_ASSERT(m_insideFrame, L"Frame not begun.");
	T_CAPTURE_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_threadFrame, L"Call thread inconsistent.");

	m_renderView->endFrame();
	m_insideFrame = false;
}

void RenderViewVrfy::present()
{
	T_CAPTURE_TRACE(L"present");
	T_CAPTURE_ASSERT(!m_insideFrame, L"Cannot present inside beginFrame/endFrame.");
	T_CAPTURE_ASSERT(m_threadFrame, L"Cannot present multiple times without rendering.");
	T_CAPTURE_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_threadFrame, L"Call thread inconsistent.");

	m_renderView->present();
	m_threadFrame = nullptr;
}

bool RenderViewVrfy::beginPass(const Clear* clear, uint32_t load, uint32_t store)
{
	T_CAPTURE_TRACE(L"beginPass");
	T_CAPTURE_ASSERT(!m_insidePass, L"Already inside pass.");
	T_CAPTURE_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_threadFrame, L"Call thread inconsistent.");

	if (!m_renderView->beginPass(clear, load, store))
		return false;

	m_insidePass = true;
	return true;
}

bool RenderViewVrfy::beginPass(IRenderTargetSet* renderTargetSet, const Clear* clear, uint32_t load, uint32_t store)
{
	T_CAPTURE_TRACE(L"beginPass");
	T_CAPTURE_ASSERT(!m_insidePass, L"Already inside pass.");
	T_CAPTURE_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_threadFrame, L"Call thread inconsistent.");

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
	T_CAPTURE_TRACE(L"beginPass");
	T_CAPTURE_ASSERT(!m_insidePass, L"Already inside pass.");
	T_CAPTURE_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_threadFrame, L"Call thread inconsistent.");

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
	T_CAPTURE_TRACE(L"endPass");
	T_CAPTURE_ASSERT(m_insidePass, L"Not inside pass.");
	T_CAPTURE_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_threadFrame, L"Call thread inconsistent.");

	m_renderView->endPass();
	m_insidePass = false;
}

void RenderViewVrfy::draw(const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	T_CAPTURE_TRACE(L"draw");
	T_CAPTURE_ASSERT(m_insidePass, L"Cannot draw outside of beginPass/endPass.");
	T_CAPTURE_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_threadFrame, L"Call thread inconsistent.");

	ProgramVrfy* programVrfy = dynamic_type_cast< ProgramVrfy* >(program);
	T_CAPTURE_ASSERT(programVrfy, L"Incorrect program type.");

	if (!programVrfy)
		return;

	T_CAPTURE_ASSERT(programVrfy->m_program, L"Trying to draw with destroyed program.");
	T_CAPTURE_ASSERT(vertexBuffer, L"No vertex buffer.");

	if (!vertexBuffer)
		return;

	//VertexBufferVrfy* vb = checked_type_cast< VertexBufferVrfy* >(vertexBuffer);
	//IndexBufferVrfy* ib = checked_type_cast< IndexBufferVrfy* >(indexBuffer);

	// Validate draw call.
	uint32_t vertexCount = 0;
	switch (primitives.type)
	{
	case PrimitiveType::Points:
		vertexCount = primitives.count;
		break;

	case PrimitiveType::LineStrip:
		T_ASSERT(0);
		break;

	case PrimitiveType::Lines:
		vertexCount = primitives.count * 2;
		break;

	case PrimitiveType::TriangleStrip:
		vertexCount = primitives.count + 2;
		break;

	case PrimitiveType::Triangles:
		vertexCount = primitives.count * 3;
		break;
	}

	//if (primitives.indexed)
	//{
	//	T_CAPTURE_ASSERT(ib, L"Drawing indexed primitives but no index buffer.");
	//	if (!ib)
	//		return;

	//	uint32_t maxVertexCount = ib->getBufferSize();
	//	if (ib->getIndexType() == ItUInt16)
	//		maxVertexCount /= 2;
	//	else
	//		maxVertexCount /= 4;

	//	T_CAPTURE_ASSERT(primitives.offset + vertexCount <= maxVertexCount, L"Trying to draw more primitives than size of index buffer.");
	//}
	//else
	//{
	//	T_CAPTURE_ASSERT(!ib, L"Drawing non-indexed primitives but index buffer provided.");

	//	uint32_t maxVertexCount = vb->getBufferSize() / vb->getVertexSize();
	//	T_CAPTURE_ASSERT(primitives.offset + vertexCount <= maxVertexCount, L"Trying to draw more primitives than size of vertex buffer.");
	//}

	programVrfy->verify();

	m_renderView->draw(vertexBuffer, vertexLayout, indexBuffer, indexType, programVrfy->m_program, primitives, instanceCount);
}

void RenderViewVrfy::compute(IProgram* program, const int32_t* workSize)
{
	T_CAPTURE_TRACE(L"compute");
	T_CAPTURE_ASSERT(m_insideFrame, L"Cannot compute outside of beginFrame/endFrame.");
	T_CAPTURE_ASSERT(workSize != nullptr, L"Incorrect argument; workSize null.");
	T_CAPTURE_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_threadFrame, L"Call thread inconsistent.");

	ProgramVrfy* programVrfy = dynamic_type_cast< ProgramVrfy* >(program);
	T_CAPTURE_ASSERT(programVrfy, L"Incorrect program type.");

	if (!programVrfy)
		return;

	programVrfy->verify();

	m_renderView->compute(programVrfy->m_program, workSize);
}

bool RenderViewVrfy::copy(ITexture* destinationTexture, const Region& destinationRegion, ITexture* sourceTexture, const Region& sourceRegion)
{
	T_CAPTURE_TRACE(L"copy");
	T_CAPTURE_ASSERT(destinationTexture, L"Invalid destination texture.");
	T_CAPTURE_ASSERT(sourceTexture, L"Invalid destination texture.");
	T_CAPTURE_ASSERT(!m_insidePass, L"Cannot copy while being in an active render pass.");
	T_CAPTURE_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_threadFrame, L"Call thread inconsistent.");

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
	T_CAPTURE_TRACE(L"beginTimeQuery");

	int32_t query = m_renderView->beginTimeQuery();
	if (query < 0)
		return query;

	T_CAPTURE_ASSERT(m_queriesPending.find(query) == m_queriesPending.end(), L"Invalid query index returned from renderer.");
	m_queriesPending.insert(query);
	return query;
}

void RenderViewVrfy::endTimeQuery(int32_t query)
{
	T_CAPTURE_TRACE(L"endTimeQuery");

	if (query < 0)
		return;

	T_CAPTURE_ASSERT(m_queriesPending.find(query) != m_queriesPending.end(), L"Invalid query.");

	m_renderView->endTimeQuery(query);
	m_queriesPending.erase(query);
}

bool RenderViewVrfy::getTimeQuery(int32_t query, bool wait, double& outStart, double& outEnd) const
{
	T_CAPTURE_TRACE(L"getTimeQuery");
	return m_renderView->getTimeQuery(query, wait, outStart, outEnd);
}

void RenderViewVrfy::pushMarker(const char* const marker)
{
	T_CAPTURE_TRACE(L"pushMarker");
	T_CAPTURE_ASSERT(marker != nullptr, L"Invalid marker.");

	m_renderView->pushMarker(marker);
}

void RenderViewVrfy::popMarker()
{
	T_CAPTURE_TRACE(L"popMarker");
	m_renderView->popMarker();
}

void RenderViewVrfy::getStatistics(RenderViewStatistics& outStatistics) const
{
	m_renderView->getStatistics(outStatistics);
}

	}
}
