#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#include "Render/Vulkan/RenderViewVk.h"
#if defined(_WIN32)
#	include "Render/Vulkan/Win32/Window.h"
#endif

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewVk", RenderViewVk, IRenderView)

#if defined(_WIN32)
RenderViewVk::RenderViewVk(Window* window)
:	m_window(window)
{
	if (m_window)
		m_window->addListener(this);
}
#else
RenderViewVk::RenderViewVk()
{
}
#endif

RenderViewVk::~RenderViewVk()
{
	close();
}

bool RenderViewVk::nextEvent(RenderEvent& outEvent)
{
	return false;
}

void RenderViewVk::close()
{
}

bool RenderViewVk::reset(const RenderViewDefaultDesc& desc)
{
#if defined(_WIN32)

	// Cannot reset embedded view.
	if (!m_window)
		return false;

	m_window->removeListener(this);
	m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - Vulkan Renderer");
	m_window->addListener(this);

#endif
	return true;
}

bool RenderViewVk::reset(int32_t width, int32_t height)
{
	return false;
}

int RenderViewVk::getWidth() const
{
	return 0;
}

int RenderViewVk::getHeight() const
{
	return 0;
}

bool RenderViewVk::isActive() const
{
	return true;
}

bool RenderViewVk::isFullScreen() const
{
	return true;
}

void RenderViewVk::showCursor()
{
}

void RenderViewVk::hideCursor()
{
}

bool RenderViewVk::isCursorVisible() const
{
	return false;
}

bool RenderViewVk::setGamma(float gamma)
{
	return false;
}

void RenderViewVk::setViewport(const Viewport& viewport)
{
}

Viewport RenderViewVk::getViewport()
{
	return Viewport();
}

SystemWindow RenderViewVk::getSystemWindow()
{
	SystemWindow sw;
	return sw;
}

bool RenderViewVk::begin(EyeType eye)
{
	return true;
}

bool RenderViewVk::begin(RenderTargetSet* renderTargetSet)
{
	return true;
}

bool RenderViewVk::begin(RenderTargetSet* renderTargetSet, int renderTarget)
{
	return true;
}

void RenderViewVk::clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil)
{
}

void RenderViewVk::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
}

void RenderViewVk::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
}

void RenderViewVk::end()
{
}

void RenderViewVk::present()
{
}

void RenderViewVk::pushMarker(const char* const marker)
{
}

void RenderViewVk::popMarker()
{
}

void RenderViewVk::getStatistics(RenderViewStatistics& outStatistics) const
{
	outStatistics.drawCalls = 0;
	outStatistics.primitiveCount = 0;
}

bool RenderViewVk::getBackBufferContent(void* buffer) const
{
	return false;
}

#if defined(_WIN32)
bool RenderViewVk::windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult)
{
	return false;
}
#endif

	}
}
