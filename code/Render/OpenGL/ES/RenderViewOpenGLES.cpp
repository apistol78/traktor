#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/OpenGL/ES/ExtensionsGLES.h"
#include "Render/OpenGL/ES/IndexBufferOpenGLES.h"
#include "Render/OpenGL/ES/ProgramOpenGLES.h"
#include "Render/OpenGL/ES/RenderSystemOpenGLES.h"
#include "Render/OpenGL/ES/RenderTargetOpenGLES.h"
#include "Render/OpenGL/ES/RenderTargetSetOpenGLES.h"
#include "Render/OpenGL/ES/RenderViewOpenGLES.h"
#include "Render/OpenGL/ES/StateCache.h"
#include "Render/OpenGL/ES/VertexBufferOpenGLES.h"
#if defined(__ANDROID__)
#	include "Render/OpenGL/ES/Android/ContextOpenGLES.h"
#elif defined(__IOS__)
#	include "Render/OpenGL/ES/iOS/ContextOpenGLES.h"
#elif defined(__EMSCRIPTEN__)
#	include "Render/OpenGL/ES/Emscripten/ContextOpenGLES.h"
#elif defined(_WIN32)
#	include "Render/OpenGL/ES/Win32/ContextOpenGLES.h"
#elif defined(__LINUX__) || defined(__RPI__)
#	include "Render/OpenGL/ES/Linux/ContextOpenGLES.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewOpenGLES", RenderViewOpenGLES, IRenderView)

RenderViewOpenGLES::RenderViewOpenGLES(ContextOpenGLES* context)
:	m_context(context)
,	m_stateCache(new StateCache())
{
#if defined(_WIN32)
	m_context->getWindow()->addListener(this);
#endif

	m_width = m_context->getWidth();
	m_height = m_context->getHeight();
}

RenderViewOpenGLES::~RenderViewOpenGLES()
{
}

bool RenderViewOpenGLES::nextEvent(RenderEvent& outEvent)
{
#if defined(_WIN32)

	bool going = true;
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		int ret = GetMessage(&msg, NULL, 0, 0);
		if (ret <= 0 || msg.message == WM_QUIT)
			going = false;

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

#elif defined(__LINUX__)

	return m_context ? m_context->getWindow()->update(outEvent) : false;

#elif defined(__IOS__) || defined(__ANDROID__)

	int32_t width = m_context->getWidth();
	int32_t height = m_context->getHeight();

	if (width != m_width || height != m_height)
	{
		log::info << L"Device screen size changed, new size " << width << L" * " << height << Endl;

		m_width = width;
		m_height = height;

		// Post a resize event as we need all systems to re-create
		// resources if necessary.
		outEvent.type = ReResize;
		outEvent.resize.width = width;
		outEvent.resize.height = height;
		return true;
	}

#endif
	return false;
}

void RenderViewOpenGLES::close()
{
#if defined(_WIN32)
	if (m_context->getWindow())
		m_context->getWindow()->removeListener(this);
#endif
	m_context = nullptr;
}

bool RenderViewOpenGLES::reset(const RenderViewDefaultDesc& desc)
{
#if defined(_WIN32) || defined(__LINUX__)
	m_context->getWindow()->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - OpenGL ES Renderer");
	if (desc.fullscreen)
		m_context->getWindow()->setFullScreenStyle();
	else
		m_context->getWindow()->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);

	m_context->reset(desc.displayMode.width, desc.displayMode.height);

	m_width = m_context->getWidth();
	m_height = m_context->getHeight();
#endif
	return true;
}

bool RenderViewOpenGLES::reset(int32_t width, int32_t height)
{
	return m_context->reset(width, height);
}

int RenderViewOpenGLES::getWidth() const
{
	return m_width;
}

int RenderViewOpenGLES::getHeight() const
{
	return m_height;
}

bool RenderViewOpenGLES::isActive() const
{
	return true;
}

bool RenderViewOpenGLES::isMinimized() const
{
	return false;
}

bool RenderViewOpenGLES::isFullScreen() const
{
#if defined(T_OPENGL_ES_HAVE_EGL)
	return false;
#else
	return true;
#endif
}

void RenderViewOpenGLES::showCursor()
{
	m_cursorVisible = true;
}

void RenderViewOpenGLES::hideCursor()
{
	m_cursorVisible = false;
}

bool RenderViewOpenGLES::isCursorVisible() const
{
	return m_cursorVisible;
}

bool RenderViewOpenGLES::setGamma(float gamma)
{
	return false;
}

void RenderViewOpenGLES::setViewport(const Viewport& viewport)
{
	T_OGL_SAFE(glViewport(
		viewport.left,
		viewport.top,
		viewport.width,
		viewport.height
	));

	T_OGL_SAFE(glDepthRangef(
		viewport.nearZ,
		viewport.farZ
	));
}

SystemWindow RenderViewOpenGLES::getSystemWindow()
{
#if defined(_WIN32)
	SystemWindow sw;
	sw.hWnd = *m_context->getWindow();
	return sw;
#elif defined(__LINUX__)
	SystemWindow sw;
	sw.display = m_context->getWindow()->getDisplay();
	sw.window = m_context->getWindow()->getWindow();
	return sw;
#else
	return SystemWindow();
#endif
}

bool RenderViewOpenGLES::beginFrame()
{
	if (!m_context->enter())
		return false;

	m_stateCache->reset();
	m_drawCalls = 0;
	m_primitiveCount = 0;

	return true;
}

void RenderViewOpenGLES::endFrame()
{
}

void RenderViewOpenGLES::present()
{
	m_context->swapBuffers();
	m_context->leave();
	m_context->deleteResources();
}

bool RenderViewOpenGLES::beginPass(const Clear* clear)
{
	m_context->bindPrimary();

	m_activeRenderTargetSet = nullptr;

	T_OGL_SAFE(glViewport(
		0,
		0,
		m_context->getWidth(),
		m_context->getHeight()
	));

	T_OGL_SAFE(glDepthRangef(
		0.0f,
		1.0f
	));

	if (clear && clear->mask != 0)
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

		GLuint cm = c_clearMask[clear->mask];

		if (cm & GL_COLOR_BUFFER_BIT)
		{
			m_stateCache->setColorMask(RenderStateOpenGL::CmAll);
			float r = clear->colors[0].getRed();
			float g = clear->colors[0].getGreen();
			float b = clear->colors[0].getBlue();
			float a = clear->colors[0].getAlpha();
			T_OGL_SAFE(glClearColor(r, g, b, a));
		}

		if (cm & GL_DEPTH_BUFFER_BIT)
		{
			m_stateCache->setDepthMask(GL_TRUE);
			T_OGL_SAFE(glClearDepthf(clear->depth));
		}

		if (cm & GL_STENCIL_BUFFER_BIT)
			T_OGL_SAFE(glClearStencil(clear->stencil));

		T_OGL_SAFE(glClear(cm));
	}

	return true;
}

bool RenderViewOpenGLES::beginPass(IRenderTargetSet* renderTargetSet, const Clear* clear, uint32_t load, uint32_t store)
{
	return beginPass(renderTargetSet, 0, clear, load, store);
}

bool RenderViewOpenGLES::beginPass(IRenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear, uint32_t load, uint32_t store)
{
	RenderTargetSetOpenGLES* rts = checked_type_cast< RenderTargetSetOpenGLES* >(renderTargetSet);

	if (!rts->bind(m_context->getPrimaryDepth(), renderTarget))
		return false;

	m_activeRenderTargetSet = rts;

	T_OGL_SAFE(glViewport(
		0,
		0,
		rts->getWidth(),
		rts->getHeight()
	));

	T_OGL_SAFE(glDepthRangef(
		0.0f,
		1.0f
	));

	if (clear && clear->mask != 0)
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

		GLuint cm = c_clearMask[clear->mask];

		if (cm & GL_COLOR_BUFFER_BIT)
		{
			m_stateCache->setColorMask(RenderStateOpenGL::CmAll);
			float r = clear->colors[0].getRed();
			float g = clear->colors[0].getGreen();
			float b = clear->colors[0].getBlue();
			float a = clear->colors[0].getAlpha();
			T_OGL_SAFE(glClearColor(r, g, b, a));
		}

		if (cm & GL_DEPTH_BUFFER_BIT)
		{
			m_stateCache->setDepthMask(GL_TRUE);
			T_OGL_SAFE(glClearDepthf(clear->depth));
		}

		if (cm & GL_STENCIL_BUFFER_BIT)
			T_OGL_SAFE(glClearStencil(clear->stencil));

		T_OGL_SAFE(glClear(cm));
	}

	return true;
}

void RenderViewOpenGLES::endPass()
{
	if (m_activeRenderTargetSet)
	{
		m_activeRenderTargetSet->setContentValid(true);
		m_activeRenderTargetSet = nullptr;
	}
}

void RenderViewOpenGLES::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	VertexBufferOpenGLES* vertexBufferGL = checked_type_cast< VertexBufferOpenGLES* >(vertexBuffer);
	IndexBufferOpenGLES* indexBufferGL = checked_type_cast< IndexBufferOpenGLES* >(indexBuffer);
	ProgramOpenGLES* programGL = checked_type_cast< ProgramOpenGLES * >(program);

	vertexBufferGL->activate(m_stateCache);

	float targetSize[2];
	float postTransform[4];
	bool invertCull;

	if (!m_activeRenderTargetSet)
	{
		targetSize[0] = float(getWidth());
		targetSize[1] = float(getHeight());

		postTransform[0] = 1.0f;
		postTransform[1] = 0.0f;
		postTransform[2] = 0.0f;
		postTransform[3] = 1.0f;

		invertCull = false;
	}
	else
	{
		targetSize[0] = (float)m_activeRenderTargetSet->getWidth();
		targetSize[1] = (float)m_activeRenderTargetSet->getHeight();

		postTransform[0] = 1.0f;
		postTransform[1] = 0.0f;
		postTransform[2] = 0.0f;
		postTransform[3] = -1.0f;

		invertCull = true;
	}

	if (!programGL->activate(m_stateCache, targetSize, postTransform, invertCull))
		return;

	GLenum primitiveType = GL_POINTS;
	GLuint vertexCount = 0;

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
		T_ASSERT(0);
	}

	if (primitives.indexed)
	{
		T_ASSERT_M (indexBufferGL, L"No index buffer");

		GLenum indexType = 0;
		GLint offsetMultiplier = 0;

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

		default:
			return;
		}

		indexBufferGL->activate(m_stateCache);

		T_OGL_SAFE(glDrawElements(
			primitiveType,
			vertexCount,
			indexType,
			(const GLubyte*)(primitives.offset * offsetMultiplier)
		));
	}
	else
	{
		T_OGL_SAFE(glDrawArrays(
			primitiveType,
			primitives.offset,
			vertexCount
		));
	}

	m_drawCalls++;
	m_primitiveCount += primitives.count;
}

void RenderViewOpenGLES::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	VertexBufferOpenGLES* vertexBufferGL = checked_type_cast< VertexBufferOpenGLES* >(vertexBuffer);
	IndexBufferOpenGLES* indexBufferGL = checked_type_cast< IndexBufferOpenGLES* >(indexBuffer);
	ProgramOpenGLES* programGL = checked_type_cast< ProgramOpenGLES * >(program);

	vertexBufferGL->activate(m_stateCache);

	float targetSize[2];
	float postTransform[4];
	bool invertCull;

	if (!m_activeRenderTargetSet)
	{
		targetSize[0] = float(getWidth());
		targetSize[1] = float(getHeight());

		postTransform[0] = 1.0f;
		postTransform[1] = 0.0f;
		postTransform[2] = 0.0f;
		postTransform[3] = 1.0f;

		invertCull = false;
	}
	else
	{
		targetSize[0] = (float)m_activeRenderTargetSet->getWidth();
		targetSize[1] = (float)m_activeRenderTargetSet->getHeight();

		postTransform[0] = 1.0f;
		postTransform[1] = 0.0f;
		postTransform[2] = 0.0f;
		postTransform[3] = -1.0f;

		invertCull = true;
	}

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
		T_ASSERT(0);
	}

	if (!programGL->activate(m_stateCache, targetSize, postTransform, invertCull))
		return;

	if (primitives.indexed)
	{
		T_ASSERT_M (indexBufferGL, L"No index buffer");

		GLenum indexType = 0;
		GLint offsetMultiplier = 0;

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

		default:
			return;
		}

		indexBufferGL->activate(m_stateCache);

		T_OGL_SAFE(glDrawElementsInstanced(
			primitiveType,
			vertexCount,
			indexType,
			(const GLubyte*)(primitives.offset * offsetMultiplier),
			instanceCount
		));
	}
	else
	{
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

void RenderViewOpenGLES::compute(IProgram* program, const int32_t* workSize)
{
}

bool RenderViewOpenGLES::copy(ITexture* destinationTexture, const Region& destinationRegion, ITexture* sourceTexture, const Region& sourceRegion)
{
	return false;
}

int32_t RenderViewOpenGLES::beginTimeQuery()
{
	return 0;
}

void RenderViewOpenGLES::endTimeQuery(int32_t query)
{
}

bool RenderViewOpenGLES::getTimeQuery(int32_t query, bool wait, double& outDuration) const
{
	return false;
}

void RenderViewOpenGLES::pushMarker(const char* const marker)
{
#if defined(__IOS__)
	glPushGroupMarkerEXT(0, marker);
#endif
}

void RenderViewOpenGLES::popMarker()
{
#if defined(__IOS__)
	glPopGroupMarkerEXT();
#endif
}

void RenderViewOpenGLES::getStatistics(RenderViewStatistics& outStatistics) const
{
	outStatistics.drawCalls = m_drawCalls;
	outStatistics.primitiveCount = m_primitiveCount;
}

#if defined(_WIN32)
bool RenderViewOpenGLES::windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult)
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

		if (width <= 0 || height <= 0)
			return false;

		if (width != m_context->getWidth() || height != m_context->getHeight())
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
