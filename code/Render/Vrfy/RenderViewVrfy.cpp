/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Thread/ThreadManager.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Vrfy/AccelerationStructureVrfy.h"
#include "Render/Vrfy/BufferVrfy.h"
#include "Render/Vrfy/Error.h"
#include "Render/Vrfy/ProgramVrfy.h"
#include "Render/Vrfy/RenderTargetSetVrfy.h"
#include "Render/Vrfy/RenderViewVrfy.h"
#include "Render/Vrfy/TextureVrfy.h"
#include "Render/Vrfy/VertexLayoutVrfy.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewVrfy", RenderViewVrfy, IRenderView)

RenderViewVrfy::RenderViewVrfy(const RenderViewDesc& desc, IRenderSystem* renderSystem, IRenderView* renderView)
:	m_desc(desc)
,	m_renderSystem(renderSystem)
,	m_renderView(renderView)
{
}

RenderViewVrfy::~RenderViewVrfy()
{
	T_CAPTURE_ASSERT(m_closed, L"Render view not properly closed.");
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
	m_closed = true;
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

uint32_t RenderViewVrfy::getDisplay() const
{
	T_CAPTURE_TRACE(L"getDisplay");
	return m_renderView->getDisplay();
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

	const BufferViewVrfy* vbv = checked_type_cast< const BufferViewVrfy* >(vertexBuffer);
	const BufferViewVrfy* ibv = checked_type_cast< const BufferViewVrfy* >(indexBuffer);
	const VertexLayoutVrfy* vl = checked_type_cast< const VertexLayoutVrfy* >(vertexLayout);

	// Validate draw call.
	const uint32_t vertexCount = primitives.getVertexCount();

	if (primitives.indexed)
	{
		T_CAPTURE_ASSERT(ibv, L"Drawing indexed primitives but no index buffer view.");
		if (!ibv)
			return;

		const BufferVrfy* ib = ibv->getBuffer();
		const uint32_t maxVertexCount =  ib->getBufferSize() / ((indexType == IndexType::UInt16) ? 2 : 4);

		T_CAPTURE_ASSERT(primitives.offset + vertexCount <= maxVertexCount, L"Trying to draw more primitives than size of index buffer.");
	}
	else if (vbv && vl)
	{
		T_CAPTURE_ASSERT(!ibv, L"Drawing non-indexed primitives but index buffer provided.");

		const BufferVrfy* vb = vbv->getBuffer();
		const uint32_t maxVertexCount = vb->getBufferSize() / vl->getVertexSize();
		T_CAPTURE_ASSERT(primitives.offset + vertexCount <= maxVertexCount, L"Trying to draw more primitives than size of vertex buffer.");
	}

	programVrfy->verify();

	const IBufferView* wrappedVertexView = vbv != nullptr ? vbv->getWrappedBufferView() : nullptr;
	const IBufferView* wrappedIndexView = ibv != nullptr ? ibv->getWrappedBufferView() : nullptr;
	const IVertexLayout* wrappedVertexLayout = vl != nullptr ? vl->getWrappedVertexLayout() : nullptr;

	m_renderView->draw(wrappedVertexView, wrappedVertexLayout, wrappedIndexView, indexType, programVrfy->m_program, primitives, instanceCount);
}

void RenderViewVrfy::drawIndirect(const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, IProgram* program, PrimitiveType primitiveType, const IBufferView* drawBuffer, uint32_t drawOffset, uint32_t drawCount)
{
	T_CAPTURE_TRACE(L"drawIndirect");
	T_CAPTURE_ASSERT(m_insidePass, L"Cannot draw outside of beginPass/endPass.");
	T_CAPTURE_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_threadFrame, L"Call thread inconsistent.");
	T_CAPTURE_ASSERT(drawBuffer, L"Need draw buffer when drawIndirect.");

	ProgramVrfy* programVrfy = dynamic_type_cast< ProgramVrfy* >(program);
	T_CAPTURE_ASSERT(programVrfy, L"Incorrect program type.");
	if (!programVrfy)
		return;
	T_CAPTURE_ASSERT(programVrfy->m_program, L"Trying to draw with destroyed program.");

	const BufferViewVrfy* vbv = checked_type_cast< const BufferViewVrfy* >(vertexBuffer);
	const BufferViewVrfy* ibv = checked_type_cast< const BufferViewVrfy* >(indexBuffer);
	const BufferViewVrfy* dbv = checked_type_cast< const BufferViewVrfy* >(drawBuffer);

	if (!dbv)
		return;

	const VertexLayoutVrfy* vl = checked_type_cast< const VertexLayoutVrfy* >(vertexLayout);
	if (!vl)
		return;

	programVrfy->verify();

	const IBufferView* wrappedVertexView = vbv != nullptr ? vbv->getWrappedBufferView() : nullptr;
	const IBufferView* wrappedIndexView = ibv != nullptr ? ibv->getWrappedBufferView() : nullptr;
	const IBufferView* wrappedDrawView = dbv != nullptr ? dbv->getWrappedBufferView() : nullptr;

	m_renderView->drawIndirect(wrappedVertexView, vl->getWrappedVertexLayout(), wrappedIndexView, indexType, programVrfy->m_program, primitiveType, wrappedDrawView, drawOffset, drawCount);
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

void RenderViewVrfy::computeIndirect(IProgram* program, const IBufferView* workBuffer, uint32_t workOffset)
{
	T_CAPTURE_TRACE(L"computeIndirect");
	T_CAPTURE_ASSERT(m_insideFrame, L"Cannot compute outside of beginFrame/endFrame.");
	T_CAPTURE_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_threadFrame, L"Call thread inconsistent.");

	const BufferViewVrfy* wbv = checked_type_cast< const BufferViewVrfy* >(workBuffer);
	if (!wbv)
		return;

	ProgramVrfy* programVrfy = dynamic_type_cast< ProgramVrfy* >(program);
	T_CAPTURE_ASSERT(programVrfy, L"Incorrect program type.");

	if (!programVrfy)
		return;

	programVrfy->verify();

	const IBufferView* wrappedWorldView = wbv != nullptr ? wbv->getWrappedBufferView() : nullptr;

	m_renderView->computeIndirect(programVrfy->m_program, wrappedWorldView, workOffset);
}

void RenderViewVrfy::barrier(Stage from, Stage to, ITexture* written, uint32_t writtenMip)
{
	T_CAPTURE_TRACE(L"barier");
	T_CAPTURE_ASSERT(m_insideFrame, L"Cannot insert barrier outside of beginFrame/endFrame.");
	T_CAPTURE_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_threadFrame, L"Call thread inconsistent.");

	ITexture* writtenUnwrapped = nullptr;
	if (written != nullptr)
	{
		if (TextureVrfy* textureVrfy = dynamic_type_cast< TextureVrfy* >(written))
			writtenUnwrapped = textureVrfy->getTexture();
		else
			T_FATAL_ERROR;
	}

	m_renderView->barrier(from, to, writtenUnwrapped, writtenMip);
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

	if (TextureVrfy* textureVrfy = dynamic_type_cast< TextureVrfy* >(destinationTexture))
		destinationTextureUnwrapped = textureVrfy->getTexture();
	else
		T_FATAL_ERROR;

	if (TextureVrfy* textureVrfy = dynamic_type_cast< TextureVrfy* >(sourceTexture))
		sourceTextureUnwrapped = textureVrfy->getTexture();
	else
		T_FATAL_ERROR;

	T_CAPTURE_ASSERT(destinationTextureUnwrapped, L"Using destroyed texture as destination.");
	T_CAPTURE_ASSERT(sourceTextureUnwrapped, L"Using destroyed texture as source.");
	if (destinationTextureUnwrapped == nullptr || sourceTextureUnwrapped == nullptr)
		return false;

	return m_renderView->copy(destinationTextureUnwrapped, destinationRegion, sourceTextureUnwrapped, sourceRegion);
}

void RenderViewVrfy::writeAccelerationStructure(IAccelerationStructure* accelerationStructure, const AlignedVector< IAccelerationStructure::Instance >& instances)
{
	T_CAPTURE_TRACE(L"writeAccelerationStructure");
	T_CAPTURE_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_threadFrame, L"Call thread inconsistent.");

	AccelerationStructureVrfy* as = dynamic_type_cast< AccelerationStructureVrfy* >(accelerationStructure);
	T_CAPTURE_ASSERT(as != nullptr, L"Invalid acceleration structure (TLAS).");
	if (!as)
		return;

	T_CAPTURE_ASSERT(as->getWrappedAS(), L"Cannot write TLAS; TLAS destroyed.");
	if (!as->getWrappedAS())
		return;

	AlignedVector< IAccelerationStructure::Instance > unwrappedInstances;
	unwrappedInstances.reserve(instances.size());

	for (const auto& instance : instances)
	{
		const AccelerationStructureVrfy* blas = dynamic_type_cast< const AccelerationStructureVrfy* >(instance.blas);
		T_CAPTURE_ASSERT(blas != nullptr, L"Invalid acceleration structure (instance BLAS).");
		if (!blas)
			continue;

		T_CAPTURE_ASSERT(blas->getWrappedAS(), L"Cannot write BLAS into TLAS; BLAS destroyed.");
		if (!blas->getWrappedAS())
			continue;

		const BufferVrfy* bv = dynamic_type_cast< const BufferVrfy* >(instance.perPrimitiveVec4);

		unwrappedInstances.push_back({
			blas->getWrappedAS(),
			(bv != nullptr) ? bv->getWrappedBuffer() : nullptr,
			instance.transform
		});
	}

	m_renderView->writeAccelerationStructure(as->getWrappedAS(), unwrappedInstances);
}

void RenderViewVrfy::writeAccelerationStructure(IAccelerationStructure* accelerationStructure, const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, const AlignedVector< Primitives >& primitives)
{
	T_CAPTURE_TRACE(L"writeAccelerationStructure");
	T_CAPTURE_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_threadFrame, L"Call thread inconsistent.");

	AccelerationStructureVrfy* as = dynamic_type_cast< AccelerationStructureVrfy* >(accelerationStructure);
	T_CAPTURE_ASSERT(as != nullptr, L"Invalid acceleration structure (TLAS).");
	if (!as)
		return;

	T_CAPTURE_ASSERT(as->getWrappedAS(), L"Cannot write TLAS; TLAS destroyed.");
	if (!as->getWrappedAS())
		return;

	T_CAPTURE_ASSERT(vertexBuffer, L"Missing vertex buffer.");
	T_CAPTURE_ASSERT(indexBuffer, L"Missing index buffer.");
	T_CAPTURE_ASSERT(vertexLayout, L"Missing vertex layout.");
	if (!vertexBuffer || !indexBuffer || !vertexLayout)
		return;

	const BufferViewVrfy* vbv = checked_type_cast< const BufferViewVrfy* >(vertexBuffer);
	const BufferViewVrfy* ibv = checked_type_cast< const BufferViewVrfy* >(indexBuffer);
	const VertexLayoutVrfy* vl = checked_type_cast< const VertexLayoutVrfy* >(vertexLayout);

	m_renderView->writeAccelerationStructure(as->getWrappedAS(), vbv->getWrappedBufferView(), vl->getWrappedVertexLayout(), ibv->getWrappedBufferView(), indexType, primitives);
}

int32_t RenderViewVrfy::beginTimeQuery()
{
	T_CAPTURE_TRACE(L"beginTimeQuery");

	const int32_t query = m_renderView->beginTimeQuery();
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

void RenderViewVrfy::pushMarker(const std::wstring& marker)
{
	T_CAPTURE_TRACE(L"pushMarker");
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
