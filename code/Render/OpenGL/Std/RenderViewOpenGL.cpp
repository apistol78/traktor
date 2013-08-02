#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/VertexBufferOpenGL.h"
#include "Render/OpenGL/IndexBufferOpenGL.h"
#include "Render/OpenGL/Std/RenderViewOpenGL.h"
#include "Render/OpenGL/Std/RenderSystemOpenGL.h"
#include "Render/OpenGL/Std/ProgramOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetSetOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetOpenGL.h"

#if defined(__APPLE__)
#	include "Render/OpenGL/Std/OsX/CGLWindow.h"
#	include "Render/OpenGL/Std/OsX/CGLWrapper.h"
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

struct RenderEventTypePred
{
	RenderEventType m_type;

	RenderEventTypePred(RenderEventType type)
	:	m_type(type)
	{
	}

	bool operator () (const RenderEvent& evt) const
	{
		return evt.type == m_type;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewOpenGL", RenderViewOpenGL, IRenderView)

#if defined(_WIN32)

RenderViewOpenGL::RenderViewOpenGL(
	const RenderViewDesc desc,
	Window* window,
	ContextOpenGL* renderContext,
	ContextOpenGL* resourceContext
)
:	m_window(window)
,	m_renderContext(renderContext)
,	m_resourceContext(resourceContext)
,	m_cursorVisible(true)
,	m_waitVBlank(false)
,	m_drawCalls(0)
,	m_primitiveCount(0)
{
	m_primaryTargetDesc.multiSample = desc.multiSample;
	m_primaryTargetDesc.createDepthStencil = bool(desc.depthBits > 0 || desc.stencilBits > 0);
	m_primaryTargetDesc.usingPrimaryDepthStencil = false;
	m_primaryTargetDesc.preferTiled = false;
	m_primaryTargetDesc.ignoreStencil = bool(desc.stencilBits == 0);
	m_waitVBlank = desc.waitVBlank;

	if (m_window)
		m_window->addListener(this);
}

#elif defined(__APPLE__)

RenderViewOpenGL::RenderViewOpenGL(
	const RenderViewDesc desc,
	void* windowHandle,
	ContextOpenGL* renderContext,
	ContextOpenGL* resourceContext
)
:	m_windowHandle(windowHandle)
,	m_renderContext(renderContext)
,	m_resourceContext(resourceContext)
,	m_cursorVisible(true)
,	m_waitVBlank(false)
{
	m_primaryTargetDesc.multiSample = desc.multiSample;
	m_primaryTargetDesc.createDepthStencil = bool(desc.depthBits > 0 || desc.stencilBits > 0);
	m_primaryTargetDesc.usingPrimaryDepthStencil = false;
	m_primaryTargetDesc.preferTiled = false;
	m_primaryTargetDesc.ignoreStencil = bool(desc.stencilBits == 0);
	m_waitVBlank = desc.waitVBlank;
}

#elif defined(__LINUX__)

RenderViewOpenGL::RenderViewOpenGL(
	const RenderViewDesc desc,
	Window* window,
	ContextOpenGL* renderContext,
	ContextOpenGL* resourceContext
)
:   m_window(window)
,	m_renderContext(renderContext)
,	m_resourceContext(resourceContext)
,	m_cursorVisible(true)
,	m_waitVBlank(false)
{
	m_primaryTargetDesc.multiSample = desc.multiSample;
	m_primaryTargetDesc.createDepthStencil = bool(desc.depthBits > 0 || desc.stencilBits > 0);
	m_primaryTargetDesc.usingPrimaryDepthStencil = false;
	m_primaryTargetDesc.preferTiled = false;
	m_primaryTargetDesc.ignoreStencil = bool(desc.stencilBits == 0);
	m_waitVBlank = desc.waitVBlank;
}

#endif

RenderViewOpenGL::~RenderViewOpenGL()
{
	close();
}

bool RenderViewOpenGL::createPrimaryTarget()
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

	m_primaryTargetDesc.count = 1;
	m_primaryTargetDesc.width = m_renderContext->getWidth();
	m_primaryTargetDesc.height = m_renderContext->getHeight();
	m_primaryTargetDesc.targets[0].format = TfR8G8B8A8;

	if (m_primaryTargetDesc.width > 0 && m_primaryTargetDesc.height > 0)
	{
		m_primaryTarget = new RenderTargetSetOpenGL(m_resourceContext);
		if (!m_primaryTarget->create(m_primaryTargetDesc))
			return false;
	}

	return true;
}

bool RenderViewOpenGL::nextEvent(RenderEvent& outEvent)
{
#if defined(_WIN32)

	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (!m_eventQueue.empty())
	{
		outEvent = m_eventQueue.front();
		m_eventQueue.pop_front();
		return true;
	}
	else
		return false;

#elif defined(__APPLE__)

	return cglwUpdateWindow(m_windowHandle, outEvent);

#elif defined(__LINUX__)

    if (m_window)
        return m_window->update(outEvent);

#endif

	return false;
}

void RenderViewOpenGL::close()
{
#if defined(_WIN32)

	if (m_window)
	{
		m_window->removeListener(this);
		m_window = 0;
	}

#endif

	safeDestroy(m_primaryTarget);
	safeDestroy(m_renderContext);
}

bool RenderViewOpenGL::reset(const RenderViewDefaultDesc& desc)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

	safeDestroy(m_primaryTarget);

#if defined(_WIN32)

	m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - OpenGL Renderer");

	if (desc.fullscreen)
		m_window->setFullScreenStyle(desc.displayMode.width, desc.displayMode.height);
	else
		m_window->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);

#elif defined(__APPLE__)

	cglwModifyWindow(m_windowHandle, desc.displayMode, desc.fullscreen);

#elif defined(__LINUX__)

	m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - OpenGL Renderer");

	if (desc.fullscreen)
		m_window->setFullScreenStyle(desc.displayMode.width, desc.displayMode.height);
	else
		m_window->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);

#endif

	// Update render context to ensure dimensions are set.
	m_renderContext->update();

	// Re-create primary FBO target.
	m_primaryTargetDesc.width = m_renderContext->getWidth();
	m_primaryTargetDesc.height = m_renderContext->getHeight();
	m_primaryTargetDesc.multiSample = desc.multiSample;

	log::info << L"Creating primary target " << m_primaryTargetDesc.width << L" x " << m_primaryTargetDesc.height << Endl;
	if (m_primaryTargetDesc.width > 0 && m_primaryTargetDesc.height > 0)
	{
		m_primaryTarget = new RenderTargetSetOpenGL(m_resourceContext);
		if (!m_primaryTarget->create(m_primaryTargetDesc))
			return false;
	}

	m_waitVBlank = desc.waitVBlank;

	return true;
}

bool RenderViewOpenGL::reset(int32_t width, int32_t height)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

	safeDestroy(m_primaryTarget);

	// Update render context to ensure dimensions are set.
	m_renderContext->update();

	// Re-create primary FBO target.
	m_primaryTargetDesc.width = m_renderContext->getWidth();
	m_primaryTargetDesc.height = m_renderContext->getHeight();

	m_primaryTarget = new RenderTargetSetOpenGL(m_resourceContext);
	m_primaryTarget->create(m_primaryTargetDesc);

	return true;
}

int RenderViewOpenGL::getWidth() const
{
	return m_renderContext->getWidth();
}

int RenderViewOpenGL::getHeight() const
{
	return m_renderContext->getHeight();
}

bool RenderViewOpenGL::isActive() const
{
#if defined(__APPLE__)
	if (!m_windowHandle)
		return false;

	return cglwIsActive(m_windowHandle);
#else
	return true;
#endif
}

bool RenderViewOpenGL::isFullScreen() const
{
#if defined(__APPLE__)
	if (!m_windowHandle)
		return false;

	return cglwIsFullscreen(m_windowHandle);
#elif defined(__LINUX__)
	return m_window->isFullScreen();
#else
	return false;
#endif
}

void RenderViewOpenGL::showCursor()
{
	if (!m_cursorVisible)
	{
#if defined(__APPLE__)
	cglwSetCursorVisible(m_windowHandle, true);
#elif defined(__LINUX__)
		m_window->showCursor();
#endif
		m_cursorVisible = true;
	}
}

void RenderViewOpenGL::hideCursor()
{
	if (m_cursorVisible)
	{
#if defined(__APPLE__)
		cglwSetCursorVisible(m_windowHandle, false);
#elif defined(__LINUX__)
		m_window->hideCursor();
#endif
		m_cursorVisible = false;
	}
}

bool RenderViewOpenGL::setGamma(float gamma)
{
	return false;
}

void RenderViewOpenGL::setViewport(const Viewport& viewport)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_renderContext);

	T_OGL_SAFE(glViewport(
		viewport.left,
		viewport.top,
		viewport.width,
		viewport.height
	));

	T_OGL_SAFE(glDepthRange(
		viewport.nearZ,
		viewport.farZ
	));
}

Viewport RenderViewOpenGL::getViewport()
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_renderContext);

	GLint ext[4];
	T_OGL_SAFE(glGetIntegerv(GL_VIEWPORT, ext));

	GLfloat range[2];
	T_OGL_SAFE(glGetFloatv(GL_DEPTH_RANGE, range));

	Viewport viewport;
	viewport.left = ext[0];
	viewport.top = ext[1];
	viewport.width = ext[2];
	viewport.height = ext[3];
	viewport.nearZ = range[0];
	viewport.farZ = range[1];

	return viewport;
}

SystemWindow RenderViewOpenGL::getSystemWindow()
{
	SystemWindow sw;
#if defined(_WIN32)
	sw.hWnd = *m_window;
#elif defined(__LINUX__)
	sw.display = m_window->getDisplay();
	sw.window = m_window->getWindow();
#endif
	return sw;
}

bool RenderViewOpenGL::begin(EyeType eye)
{
	if (!m_primaryTarget)
		return false;

	if (!m_renderContext->enter())
		return false;

	m_drawCalls = 0;
	m_primitiveCount = 0;

	return begin(m_primaryTarget, 0);
}

bool RenderViewOpenGL::begin(RenderTargetSet* renderTargetSet)
{
	TargetScope ts;
	ts.renderTargetSet = checked_type_cast< RenderTargetSetOpenGL* >(renderTargetSet);
	ts.renderTarget = -1;
	m_targetStack.push_back(ts);

	ts.renderTargetSet->bind(m_renderContext, m_primaryTarget->getDepthBuffer());
	return true;
}

bool RenderViewOpenGL::begin(RenderTargetSet* renderTargetSet, int renderTarget)
{
	TargetScope ts;
	ts.renderTargetSet = checked_type_cast< RenderTargetSetOpenGL* >(renderTargetSet);
	ts.renderTarget = renderTarget;
	m_targetStack.push_back(ts);

	ts.renderTargetSet->bind(m_renderContext, m_primaryTarget->getDepthBuffer(), ts.renderTarget);
	return true;
}

void RenderViewOpenGL::clear(uint32_t clearMask, const Color4f* color, float depth, int32_t stencil)
{
	const GLuint c_clearMask[] =
	{
		0,
		GL_COLOR_BUFFER_BIT,
		GL_DEPTH_BUFFER_BIT,
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
		GL_STENCIL_BUFFER_BIT,
		GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
		GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT
	};

	GLuint cm = c_clearMask[clearMask];

	if (cm & GL_COLOR_BUFFER_BIT)
	{
		float r = color->getRed();
		float g = color->getGreen();
		float b = color->getBlue();
		float a = color->getAlpha();
		T_OGL_SAFE(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
		T_OGL_SAFE(glClearColor(r, g, b, a));
	}

	if (cm & GL_DEPTH_BUFFER_BIT)
	{
		T_OGL_SAFE(glDepthMask(GL_TRUE));
		T_OGL_SAFE(glClearDepth(depth));
	}

	if (cm & GL_STENCIL_BUFFER_BIT)
	{
		T_OGL_SAFE(glStencilMask(~0UL));
		T_OGL_SAFE(glClearStencil(stencil));
	}

	T_OGL_SAFE(glClear(cm));
}

void RenderViewOpenGL::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	VertexBufferOpenGL* vertexBufferGL = checked_type_cast< VertexBufferOpenGL* >(vertexBuffer);
	IndexBufferOpenGL* indexBufferGL = checked_type_cast< IndexBufferOpenGL* >(indexBuffer);
	ProgramOpenGL* programGL = checked_type_cast< ProgramOpenGL * >(program);

	const TargetScope& ts = m_targetStack.back();
	float targetSize[] =
	{
		float(ts.renderTargetSet->getWidth()),
		float(ts.renderTargetSet->getHeight())
	};

	vertexBufferGL->activate(
		programGL->getAttributeLocs()
	);

	GLenum primitiveType;
	GLuint vertexCount;

	switch (primitives.type)
	{
	case PtPoints:
		primitiveType = GL_POINTS;
		vertexCount = primitives.count;
		break;

	case PtLineStrip:
		T_ASSERT_M (0, L"PtLineStrip unsupported");
		break;

	case PtLines:
		primitiveType = GL_LINES;
		vertexCount = primitives.count * 2;
		break;

	case PtTriangleStrip:
		primitiveType = GL_TRIANGLE_STRIP;
		vertexCount = primitives.count + 2;
		break;

	case PtTriangles:
		primitiveType = GL_TRIANGLES;
		vertexCount = primitives.count * 3;
		break;

	default:
		T_ASSERT (0);
	}

	if (primitives.indexed)
	{
		T_ASSERT_M (indexBufferGL, L"No index buffer");

		GLenum indexType;
		GLint offsetMultiplier;

		switch (indexBufferGL->getIndexType())
		{
		case ItUInt16:
			indexType = GL_UNSIGNED_SHORT;
			offsetMultiplier = 2;
			break;

		case ItUInt32:
			indexType = GL_UNSIGNED_INT;
			offsetMultiplier = 4;
			break;
		}

		indexBufferGL->bind();

		if (!programGL->activate(m_renderContext, targetSize))
			return;

		const GLubyte* indices = static_cast< const GLubyte* >(indexBufferGL->getIndexData()) + primitives.offset * offsetMultiplier;
		T_OGL_SAFE(glDrawRangeElements(
			primitiveType,
			primitives.minIndex,
			primitives.maxIndex,
			vertexCount,
			indexType,
			indices
		));
	}
	else
	{
		if (!programGL->activate(m_renderContext, targetSize))
			return;

		T_OGL_SAFE(glDrawArrays(
			primitiveType,
			primitives.offset,
			vertexCount
		));
	}

	m_drawCalls++;
	m_primitiveCount += primitives.count;
}

void RenderViewOpenGL::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	VertexBufferOpenGL* vertexBufferGL = checked_type_cast< VertexBufferOpenGL* >(vertexBuffer);
	IndexBufferOpenGL* indexBufferGL = checked_type_cast< IndexBufferOpenGL* >(indexBuffer);
	ProgramOpenGL* programGL = checked_type_cast< ProgramOpenGL * >(program);

	const TargetScope& ts = m_targetStack.back();
	float targetSize[] =
	{
		float(ts.renderTargetSet->getWidth()),
		float(ts.renderTargetSet->getHeight())
	};

	vertexBufferGL->activate(
		programGL->getAttributeLocs()
	);

	GLenum primitiveType;
	GLuint vertexCount;

	switch (primitives.type)
	{
	case PtPoints:
		primitiveType = GL_POINTS;
		vertexCount = primitives.count;
		break;

	case PtLineStrip:
		T_ASSERT_M (0, L"PtLineStrip unsupported");
		break;

	case PtLines:
		primitiveType = GL_LINES;
		vertexCount = primitives.count * 2;
		break;

	case PtTriangleStrip:
		primitiveType = GL_TRIANGLE_STRIP;
		vertexCount = primitives.count + 2;
		break;

	case PtTriangles:
		primitiveType = GL_TRIANGLES;
		vertexCount = primitives.count * 3;
		break;

	default:
		T_ASSERT (0);
	}

	if (primitives.indexed)
	{
		T_ASSERT_M (indexBufferGL, L"No index buffer");

		GLenum indexType;
		GLint offsetMultiplier;

		switch (indexBufferGL->getIndexType())
		{
		case ItUInt16:
			indexType = GL_UNSIGNED_SHORT;
			offsetMultiplier = 2;
			break;

		case ItUInt32:
			indexType = GL_UNSIGNED_INT;
			offsetMultiplier = 4;
			break;
		}

		indexBufferGL->bind();

		if (!programGL->activate(m_renderContext, targetSize))
			return;

		const GLubyte* indices = static_cast< const GLubyte* >(indexBufferGL->getIndexData()) + primitives.offset * offsetMultiplier;
		T_OGL_SAFE(glDrawElementsInstanced(
			primitiveType,
			vertexCount,
			indexType,
			indices,
			instanceCount
		));
	}
	else
	{
		if (!programGL->activate(m_renderContext, targetSize))
			return;

		T_OGL_SAFE(glDrawArraysInstanced(
			primitiveType,
			primitives.offset,
			vertexCount,
			instanceCount
		));
	}

	m_drawCalls++;
	m_primitiveCount += primitives.count * instanceCount;
}

void RenderViewOpenGL::end()
{
	T_ASSERT (!m_targetStack.empty());

	TargetScope ts = m_targetStack.back();
	m_targetStack.pop_back();

	if (!m_targetStack.empty())
	{
		ts = m_targetStack.back();
		if (ts.renderTarget >= 0)
			ts.renderTargetSet->bind(m_renderContext, m_primaryTarget->getDepthBuffer(), ts.renderTarget);
		else
			ts.renderTargetSet->bind(m_renderContext, m_primaryTarget->getDepthBuffer());
	}
	else
	{
		T_ASSERT (ts.renderTargetSet == m_primaryTarget);
		ts.renderTargetSet->blit();
		T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	ts.renderTargetSet->setContentValid(true);
}

void RenderViewOpenGL::present()
{
	m_renderContext->swapBuffers(m_waitVBlank);
	m_renderContext->leave();

	// Clean pending resources.
	if (m_resourceContext->enter())
	{
		m_resourceContext->deleteResources();
		m_resourceContext->leave();
	}
}

void RenderViewOpenGL::pushMarker(const char* const marker)
{
}

void RenderViewOpenGL::popMarker()
{
}

void RenderViewOpenGL::getStatistics(RenderViewStatistics& outStatistics) const
{
	outStatistics.drawCalls = m_drawCalls;
	outStatistics.primitiveCount = m_primitiveCount;
	outStatistics.duration = 0.0;
}

bool RenderViewOpenGL::getBackBufferContent(void* buffer) const
{
	return false;
}

#if defined(_WIN32)

bool RenderViewOpenGL::windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult)
{
	if (message == WM_CLOSE)
	{
		RenderEvent evt;
		evt.type = ReClose;
		m_eventQueue.push_back(evt);
	}
	else if (message == WM_SIZE)
	{
		// Remove all pending resize events.
		m_eventQueue.remove_if(RenderEventTypePred(ReResize));

		// Push new resize event if not matching current size.
		int32_t width = LOWORD(lParam);
		int32_t height = HIWORD(lParam);
		if (width != getWidth() || height != getHeight())
		{
			RenderEvent evt;
			evt.type = ReResize;
			evt.resize.width = width;
			evt.resize.height = height;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_SIZING)
	{
		RECT* rcWindowSize = (RECT*)lParam;

		int32_t width = rcWindowSize->right - rcWindowSize->left;
		int32_t height = rcWindowSize->bottom - rcWindowSize->top;

		if (width < 320)
			width = 320;
		if (height < 200)
			height = 200;

		if (wParam == WMSZ_RIGHT || wParam == WMSZ_TOPRIGHT || wParam == WMSZ_BOTTOMRIGHT)
			rcWindowSize->right = rcWindowSize->left + width;
		if (wParam == WMSZ_LEFT || wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT)
			rcWindowSize->left = rcWindowSize->right - width;

		if (wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT)
			rcWindowSize->bottom = rcWindowSize->top + height;
		if (wParam == WMSZ_TOP || wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT)
			rcWindowSize->top = rcWindowSize->bottom - height;

		outResult = TRUE;
	}
	else if (message == WM_SYSKEYDOWN)
	{
		if (wParam == VK_RETURN && (lParam & (1 << 29)) != 0)
		{
			RenderEvent evt;
			evt.type = ReToggleFullScreen;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_KEYDOWN)
	{
		if (wParam == VK_RETURN && (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0)
		{
			RenderEvent evt;
			evt.type = ReToggleFullScreen;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_SETCURSOR)
	{
		if (m_cursorVisible)
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		else
			SetCursor(NULL);
	}
	else
		return false;

	return true;
}

#endif

	}
}
