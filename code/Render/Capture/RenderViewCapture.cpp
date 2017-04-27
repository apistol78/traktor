/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/ITimeQuery.h"
#include "Render/RenderTargetSet.h"
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

	//if (m_timeQuery)
	//{
	//	m_timeQuery->begin();

	//	ProfileCapture pc;
	//	pc.name = L"Frame";
	//	pc.begin = m_timeQuery->stamp();
	//	pc.end = 0;

	//	m_timeStamps.resize(0);
	//	m_timeStamps.push_back(pc);
	//}

	m_targetDepth = 1;
	return true;
}

bool RenderViewCapture::begin(RenderTargetSet* renderTargetSet)
{
	RenderTargetSetCapture* rtsc = mandatory_non_null_type_cast< RenderTargetSetCapture* >(renderTargetSet);

	if (!m_renderView->begin(rtsc->getRenderTargetSet()))
		return false;

	++m_targetDepth;
	return true;
}

bool RenderViewCapture::begin(RenderTargetSet* renderTargetSet, int renderTarget)
{
	RenderTargetSetCapture* rtsc = mandatory_non_null_type_cast< RenderTargetSetCapture* >(renderTargetSet);

	if (!m_renderView->begin(rtsc->getRenderTargetSet(), renderTarget))
		return false;

	++m_targetDepth;
	return true;
}

void RenderViewCapture::clear(uint32_t clearMask, const Color4f* color, float depth, int32_t stencil)
{
	T_FATAL_ASSERT_M (m_targetDepth >= 1, L"Render error: Cannot clear outside of begin/end.");
	m_renderView->clear(clearMask, color, depth, stencil);
}

void RenderViewCapture::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	T_FATAL_ASSERT_M (m_targetDepth >= 1, L"Render error: Cannot draw outside of begin/end.");

	ProgramCapture* programCapture = checked_type_cast< ProgramCapture* >(program);
	if (!programCapture)
		return;

	T_FATAL_ASSERT_M (programCapture->m_program, L"Render error: Trying to draw with destroyed program.");
	T_FATAL_ASSERT_M (vertexBuffer, L"Render error: No vertex buffer.");

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
		T_ASSERT (0);
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
		T_FATAL_ASSERT_M (ib, L"Render error: Drawing indexed primitives but no index buffer.");

		uint32_t maxVertexCount = ib->getBufferSize();
		if (ib->getIndexType() == ItUInt16)
			maxVertexCount /= 2;
		else
			maxVertexCount /= 4;

		T_FATAL_ASSERT_M (primitives.offset + vertexCount <= maxVertexCount, L"Render error: Trying to draw more primitives than size of index buffer.");
	}
	else
	{
		T_FATAL_ASSERT_M (!ib, L"Render error: Drawing non-indexed primitives but index buffer provided.");

		uint32_t maxVertexCount = vb->getBufferSize() / vb->getVertexSize();
		T_FATAL_ASSERT_M (primitives.offset + vertexCount <= maxVertexCount, L"Render error: Trying to draw more primitives than size of vertex buffer.");
	}

	programCapture->verify();

	//if (m_timeQuery)
	//{
	//	ProfileCapture pc;
	//	pc.name = programCapture->m_tag.c_str();
	//	pc.begin = m_timeQuery->stamp();

		m_renderView->draw(vb->getVertexBuffer(), ib ? ib->getIndexBuffer() : 0, programCapture->m_program, primitives);

	//	pc.end = m_timeQuery->stamp();
	//	m_timeStamps.push_back(pc);
	//}
}

void RenderViewCapture::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	T_FATAL_ASSERT_M (m_targetDepth >= 1, L"Render error: Cannot draw outside of begin/end.");

	ProgramCapture* programCapture = checked_type_cast< ProgramCapture* >(program);
	if (!programCapture)
		return;

	T_FATAL_ASSERT_M (programCapture->m_program, L"Render error: Trying to draw with destroyed program.");
	T_FATAL_ASSERT_M (vertexBuffer, L"Render error: No vertex buffer.");

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
		T_ASSERT (0);
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
		T_FATAL_ASSERT_M (ib, L"Render error: Drawing indexed primitives but no index buffer.");

		uint32_t maxVertexCount = ib->getBufferSize();
		if (ib->getIndexType() == ItUInt16)
			maxVertexCount /= 2;
		else
			maxVertexCount /= 4;

		T_FATAL_ASSERT_M (primitives.offset + vertexCount <= maxVertexCount, L"Render error: Trying to draw more primitives than size of index buffer.");
	}
	else
	{
		T_FATAL_ASSERT_M (!ib, L"Render error: Drawing non-indexed primitives but index buffer provided.");

		uint32_t maxVertexCount = vb->getBufferSize() / vb->getVertexSize();
		T_FATAL_ASSERT_M (primitives.offset + vertexCount <= maxVertexCount, L"Render error: Trying to draw more primitives than size of vertex buffer.");
	}

	programCapture->verify();

	//if (m_timeQuery)
	//{
	//	ProfileCapture pc;
	//	pc.name = programCapture->m_tag.c_str();
	//	pc.begin = m_timeQuery->stamp();

		m_renderView->draw(vb->getVertexBuffer(), ib ? ib->getIndexBuffer() : 0, programCapture->m_program, primitives, instanceCount);

	//	pc.end = m_timeQuery->stamp();
	//	m_timeStamps.push_back(pc);
	//}
}

void RenderViewCapture::end()
{
	T_FATAL_ASSERT_M (m_targetDepth >= 1, L"Render error: Cannot end without begin.");

	//if (--m_targetDepth == 0)
	//{
	//	if (m_timeQuery)
	//	{
	//		ProfileCapture& pc = m_timeStamps.front();
	//		pc.end = m_timeQuery->stamp();
	//		m_timeQuery->end();
	//	}
	//}

	m_renderView->end();
}

void RenderViewCapture::present()
{
	T_FATAL_ASSERT_M (m_targetDepth >= 1, L"Render error: Cannot present inside begin/end.");

	/*
	for (std::vector< ProfileCapture >::const_iterator i = m_timeStamps.begin(); i != m_timeStamps.end(); ++i)
	{
		uint64_t timeBegin = m_timeQuery->get(i->begin);
		uint64_t timeEnd = m_timeQuery->get(i->end);
		log::info << i->name << L" -> " << (timeEnd - timeBegin) / 1000.0f << L" ms" << Endl;
	}
	*/

	m_renderView->present();
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
