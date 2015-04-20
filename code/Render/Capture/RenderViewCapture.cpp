#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Render/IRenderSystem.h"
#include "Render/ITimeQuery.h"
#include "Render/RenderTargetSet.h"
#include "Render/Capture/ProgramCapture.h"
#include "Render/Capture/RenderViewCapture.h"

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

	if (m_timeQuery)
	{
		m_timeQuery->begin();

		ProfileCapture pc;
		pc.name = L"Frame";
		pc.begin = m_timeQuery->stamp();
		pc.end = 0;

		m_timeStamps.resize(0);
		m_timeStamps.push_back(pc);
	}

	m_targetDepth = 1;
	return true;
}

bool RenderViewCapture::begin(RenderTargetSet* renderTargetSet)
{
	if (!m_renderView->begin(renderTargetSet))
		return false;

	++m_targetDepth;
	return true;
}

bool RenderViewCapture::begin(RenderTargetSet* renderTargetSet, int renderTarget)
{
	if (!m_renderView->begin(renderTargetSet, renderTarget))
		return false;

	++m_targetDepth;
	return true;
}

void RenderViewCapture::clear(uint32_t clearMask, const Color4f* color, float depth, int32_t stencil)
{
	m_renderView->clear(clearMask, color, depth, stencil);
}

void RenderViewCapture::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	ProgramCapture* programCapture = checked_type_cast< ProgramCapture* >(program);
	if (!programCapture)
		return;

	if (m_timeQuery)
	{
		ProfileCapture pc;
		pc.name = programCapture->m_tag.c_str();
		pc.begin = m_timeQuery->stamp();

		m_renderView->draw(vertexBuffer, indexBuffer, programCapture->m_program, primitives);

		pc.end = m_timeQuery->stamp();
		m_timeStamps.push_back(pc);
	}
}

void RenderViewCapture::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	ProgramCapture* programCapture = checked_type_cast< ProgramCapture* >(program);
	if (!programCapture)
		return;

	if (m_timeQuery)
	{
		ProfileCapture pc;
		pc.name = programCapture->m_tag.c_str();
		pc.begin = m_timeQuery->stamp();

		m_renderView->draw(vertexBuffer, indexBuffer, programCapture->m_program, primitives, instanceCount);

		pc.end = m_timeQuery->stamp();
		m_timeStamps.push_back(pc);
	}
}

void RenderViewCapture::end()
{
	if (--m_targetDepth == 0)
	{
		if (m_timeQuery)
		{
			ProfileCapture& pc = m_timeStamps.front();
			pc.end = m_timeQuery->stamp();
			m_timeQuery->end();
		}
	}

	m_renderView->end();
}

void RenderViewCapture::present()
{
	for (std::vector< ProfileCapture >::const_iterator i = m_timeStamps.begin(); i != m_timeStamps.end(); ++i)
	{
		uint64_t timeBegin = m_timeQuery->get(i->begin);
		uint64_t timeEnd = m_timeQuery->get(i->end);
		log::info << i->name << L" -> " << (timeEnd - timeBegin) / 1000.0f << L" ms" << Endl;
	}
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
