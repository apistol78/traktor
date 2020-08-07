#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/OpenGL/Std/Platform.h"
#include "Render/OpenGL/Std/CubeTextureOpenGL.h"
#include "Render/OpenGL/Std/IndexBufferIBO.h"
#include "Render/OpenGL/Std/ProgramOpenGL.h"
#include "Render/OpenGL/Std/RenderViewOpenGL.h"
#include "Render/OpenGL/Std/RenderSystemOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetSetOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetOpenGL.h"
#include "Render/OpenGL/Std/SimpleTextureOpenGL.h"
#include "Render/OpenGL/Std/VertexBufferOpenGL.h"

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
	const RenderViewDesc& desc,
	Window* window,
	RenderContextOpenGL* renderContext,
	ResourceContextOpenGL* resourceContext
)
:	m_window(window)
,	m_renderContext(renderContext)
,	m_resourceContext(resourceContext)
,	m_cursorVisible(true)
,	m_waitVBlanks(0)
,	m_drawCalls(0)
,	m_primitiveCount(0)
{
	m_primaryTargetDesc.count = 1;
	m_primaryTargetDesc.width = 0;
	m_primaryTargetDesc.height = 0;
	m_primaryTargetDesc.targets[0].format = TfR8G8B8A8;
	m_primaryTargetDesc.multiSample = desc.multiSample;
	m_primaryTargetDesc.createDepthStencil = bool(desc.depthBits > 0 || desc.stencilBits > 0);
	m_primaryTargetDesc.usingPrimaryDepthStencil = false;
	m_primaryTargetDesc.ignoreStencil = bool(desc.stencilBits == 0);
	m_waitVBlanks = desc.waitVBlanks;

	if (m_window)
		m_window->addListener(this);
}
#elif defined(__APPLE__)
RenderViewOpenGL::RenderViewOpenGL(
	const RenderViewDesc& desc,
	void* windowHandle,
	RenderContextOpenGL* renderContext,
	ResourceContextOpenGL* resourceContext
)
:	m_windowHandle(windowHandle)
,	m_renderContext(renderContext)
,	m_resourceContext(resourceContext)
,	m_cursorVisible(true)
,	m_waitVBlanks(0)
{
	m_primaryTargetDesc.count = 1;
	m_primaryTargetDesc.width = 0;
	m_primaryTargetDesc.height = 0;
	m_primaryTargetDesc.targets[0].format = TfR8G8B8A8;
	m_primaryTargetDesc.multiSample = desc.multiSample;
	m_primaryTargetDesc.createDepthStencil = bool(desc.depthBits > 0 || desc.stencilBits > 0);
	m_primaryTargetDesc.usingPrimaryDepthStencil = false;
	m_primaryTargetDesc.ignoreStencil = bool(desc.stencilBits == 0);
	m_waitVBlanks = desc.waitVBlanks;
}
#elif defined(__LINUX__)
RenderViewOpenGL::RenderViewOpenGL(
	const RenderViewDesc& desc,
	Window* window,
	RenderContextOpenGL* renderContext,
	ResourceContextOpenGL* resourceContext
)
:   m_window(window)
,	m_renderContext(renderContext)
,	m_resourceContext(resourceContext)
,	m_cursorVisible(true)
,	m_waitVBlanks(0)
{
	m_primaryTargetDesc.count = 1;
	m_primaryTargetDesc.width = 0;
	m_primaryTargetDesc.height = 0;
	m_primaryTargetDesc.targets[0].format = TfR8G8B8A8;
	m_primaryTargetDesc.multiSample = desc.multiSample;
	m_primaryTargetDesc.createDepthStencil = bool(desc.depthBits > 0 || desc.stencilBits > 0);
	m_primaryTargetDesc.usingPrimaryDepthStencil = false;
	m_primaryTargetDesc.ignoreStencil = bool(desc.stencilBits == 0);
	m_waitVBlanks = desc.waitVBlanks;
}
#endif

RenderViewOpenGL::~RenderViewOpenGL()
{
	close();
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
	return m_window ? m_window->update(outEvent) : false;
#else
	return false;
#endif
}

void RenderViewOpenGL::close()
{
#if defined(_WIN32)
	if (m_window)
	{
		m_window->removeListener(this);
		m_window = nullptr;
	}
#endif

	safeDestroy(m_primaryTarget);
	safeDestroy(m_renderContext);
}

bool RenderViewOpenGL::reset(const RenderViewDefaultDesc& desc)
{
	// Invalidate all created RT FBOs.
	RenderTargetSetOpenGL::ms_primaryTargetTag++;

	{
		T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_renderContext);

		// Ensure no FBO is currently bound.
		T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		T_OGL_SAFE(glFinish());
	}
	{
		T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);
		safeDestroy(m_primaryTarget);

		// Clean pending resources.
		m_resourceContext->deleteResources();

#if defined(_WIN32)
		m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - OpenGL Renderer");
		if (desc.fullscreen)
			m_window->setFullScreenStyle();
		else
			m_window->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);
#elif defined(__APPLE__)
		cglwModifyWindow(m_windowHandle, desc.displayMode, desc.fullscreen);
#elif defined(__LINUX__)
		m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - OpenGL Renderer");
		if (desc.fullscreen)
			m_window->setFullScreenStyle();
		else
			m_window->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);
#endif

		// Update render context to ensure dimensions are set.
		m_renderContext->update(
#if defined(__LINUX__)
			m_window->getWidth(),
			m_window->getHeight()
#endif
		);

		// Re-create primary FBO target.
		m_primaryTargetDesc.width = desc.displayMode.width;
		m_primaryTargetDesc.height = desc.displayMode.height;
		m_primaryTargetDesc.multiSample = desc.multiSample;

		if (m_primaryTargetDesc.width > 0 && m_primaryTargetDesc.height > 0)
		{
			m_primaryTarget = new RenderTargetSetOpenGL(m_resourceContext);
			if (!m_primaryTarget->create(m_primaryTargetDesc, nullptr))
			{
				log::error << L"Failed to create primary target" << Endl;
				return false;
			}
		}
	}

	m_waitVBlanks = desc.waitVBlanks;
	return true;
}

bool RenderViewOpenGL::reset(int32_t width, int32_t height)
{
	// Invalidate all created RT FBOs.
	RenderTargetSetOpenGL::ms_primaryTargetTag++;

	{
		T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_renderContext);

		// Ensure no FBO is currently bound.
		T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		T_OGL_SAFE(glFinish());
	}
	{
		T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);
		safeDestroy(m_primaryTarget);

		// Clean pending resources.
		m_resourceContext->deleteResources();

		// Update render context to ensure dimensions are set.
		m_renderContext->update(
#if defined(__LINUX__)
			m_window ? m_window->getWidth() : width,
			m_window ? m_window->getHeight() : height
#endif
		);

		// Re-create primary FBO target.
		m_primaryTargetDesc.width = width;
		m_primaryTargetDesc.height = height;

		if (m_primaryTargetDesc.width > 0 && m_primaryTargetDesc.height > 0)
		{
			m_primaryTarget = new RenderTargetSetOpenGL(m_resourceContext);
			if (!m_primaryTarget->create(m_primaryTargetDesc, nullptr))
			{
				log::error << L"Failed to create primary target." << Endl;
				return false;
			}
		}
	}

	return true;
}

int RenderViewOpenGL::getWidth() const
{
	return m_primaryTargetDesc.width;
}

int RenderViewOpenGL::getHeight() const
{
	return m_primaryTargetDesc.height;
}

bool RenderViewOpenGL::isActive() const
{
#if defined(__APPLE__)
	return m_windowHandle ? cglwIsActive(m_windowHandle) : false;
#elif defined(__LINUX__)
	return m_window->isActive();
#elif defined(_WIN32)
	if (m_window)
		return GetForegroundWindow() == *m_window;
	else
		return true;
#else
	return true;
#endif
}

bool RenderViewOpenGL::isMinimized() const
{
#if defined(_WIN32)
	if (m_window)
		return bool(IsIconic(*m_window) == TRUE);
	else
		return false;
#else
	return false;
#endif
}

bool RenderViewOpenGL::isFullScreen() const
{
#if defined(__APPLE__)
	return m_windowHandle ? cglwIsFullscreen(m_windowHandle) : false;
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

bool RenderViewOpenGL::isCursorVisible() const
{
	return m_cursorVisible;
}

bool RenderViewOpenGL::setGamma(float gamma)
{
	return false;
}

void RenderViewOpenGL::setViewport(const Viewport& viewport)
{
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_renderContext);

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

SystemWindow RenderViewOpenGL::getSystemWindow()
{
	SystemWindow sw;
#if defined(_WIN32)
	sw.hWnd = *m_window;
#elif defined(__APPLE__)
	sw.view = cglwGetWindowView(m_windowHandle);
#elif defined(__LINUX__)
	sw.display = m_window->getDisplay();
	sw.window = m_window->getWindow();
#endif
	return sw;
}

bool RenderViewOpenGL::beginFrame()
{
	if (!m_primaryTarget)
		return false;

	if (!m_renderContext->enter())
		return false;

	T_OGL_SAFE(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS));
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	m_drawCalls = 0;
	m_primitiveCount = 0;

	return true;
}

void RenderViewOpenGL::endFrame()
{
}

void RenderViewOpenGL::present()
{
	m_renderContext->swapBuffers(m_waitVBlanks);
	m_renderContext->leave();

	// Clean pending resources.
	if (m_resourceContext->enter())
	{
		m_resourceContext->deleteResources();
		m_resourceContext->leave();
	}
}

bool RenderViewOpenGL::beginPass(const Clear* clear)
{
	return beginPass(m_primaryTarget, 0, clear, TfColor | TfDepth, TfColor | TfDepth);
}

bool RenderViewOpenGL::beginPass(IRenderTargetSet* renderTargetSet, const Clear* clear, uint32_t load, uint32_t store)
{
	if (m_activeTarget)
		return false;

	m_activeTarget = mandatory_non_null_type_cast< RenderTargetSetOpenGL* >(renderTargetSet);
	m_activeTarget->bind(m_renderContext, m_primaryTarget->getDepthBuffer(), !m_primaryTargetDesc.ignoreStencil);

	if (clear)
	{
		if (clear->mask & CfColor)
		{
			for (int32_t i = 0; i < 8; ++i)
			{
				if (!m_activeTarget->getColorTexture(i))
					continue;

				float T_ALIGN16 cl[] = { 0.0f, 0.0f, 0.0f, 0.0f };
				clear->colors[i].storeAligned(cl);

				T_OGL_SAFE(glClearBufferfv(GL_COLOR, i, cl));
			}
		}

		if (clear->mask & CfDepth)
		{
			T_OGL_SAFE(glDepthMask(GL_TRUE));
			T_OGL_SAFE(glClearDepth(clear->depth));
			T_OGL_SAFE(glClear(GL_DEPTH_BUFFER_BIT));
		}

		if (clear->mask & CfStencil)
		{
			T_OGL_SAFE(glStencilMask(~0U));
			T_OGL_SAFE(glClearStencil(clear->stencil));
			T_OGL_SAFE(glClear(GL_STENCIL_BUFFER_BIT));
		}
	}

	return true;
}

bool RenderViewOpenGL::beginPass(IRenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear, uint32_t load, uint32_t store)
{
	if (m_activeTarget)
		return false;

	m_activeTarget = mandatory_non_null_type_cast< RenderTargetSetOpenGL* >(renderTargetSet);
	m_activeTarget->bind(m_renderContext, m_primaryTarget->getDepthBuffer(), !m_primaryTargetDesc.ignoreStencil, renderTarget);

	if (clear)
	{
		if (clear->mask & CfColor)
		{
			for (int32_t i = 0; i < 8; ++i)
			{
				if (!m_activeTarget->getColorTexture(i))
					continue;

				float T_ALIGN16 cl[] = { 0.0f, 0.0f, 0.0f, 0.0f };
				clear->colors[i].storeAligned(cl);

				T_OGL_SAFE(glClearBufferfv(GL_COLOR, i, cl));
			}
		}

		if (clear->mask & CfDepth)
		{
			T_OGL_SAFE(glDepthMask(GL_TRUE));
			T_OGL_SAFE(glClearDepth(clear->depth));
			T_OGL_SAFE(glClear(GL_DEPTH_BUFFER_BIT));
		}

		if (clear->mask & CfStencil)
		{
			T_OGL_SAFE(glStencilMask(~0U));
			T_OGL_SAFE(glClearStencil(clear->stencil));
			T_OGL_SAFE(glClear(GL_STENCIL_BUFFER_BIT));
		}
	}

	return true;
}

void RenderViewOpenGL::endPass()
{
	T_FATAL_ASSERT(m_activeTarget != nullptr);

	m_activeTarget->unbind();
	m_activeTarget->setContentValid(true);

	if (m_activeTarget == m_primaryTarget)
		m_primaryTarget->blit(m_renderContext);

	m_activeTarget = nullptr;
}

void RenderViewOpenGL::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	VertexBufferOpenGL* vertexBufferGL = checked_type_cast< VertexBufferOpenGL* >(vertexBuffer);
	IndexBufferIBO* indexBufferGL = checked_type_cast< IndexBufferIBO* >(indexBuffer);
	ProgramOpenGL* programGL = checked_type_cast< ProgramOpenGL * >(program);

	float targetSize[] =
	{
		(float)m_activeTarget->getWidth(),
		(float)m_activeTarget->getHeight()
	};

	vertexBufferGL->activate(
		m_renderContext,
		programGL->getAttributeLocs(),
		programGL->getAttributeHash()
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
		T_ASSERT(0);
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

		if (!programGL->activateRender(m_renderContext, targetSize))
			return;

		const GLubyte* indices = reinterpret_cast< const GLubyte* >(primitives.offset * offsetMultiplier);
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
		if (!programGL->activateRender(m_renderContext, targetSize))
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
	IndexBufferIBO* indexBufferGL = checked_type_cast< IndexBufferIBO* >(indexBuffer);
	ProgramOpenGL* programGL = checked_type_cast< ProgramOpenGL * >(program);

	float targetSize[] =
	{
		(float)m_activeTarget->getWidth(),
		(float)m_activeTarget->getHeight()
	};

	vertexBufferGL->activate(
		m_renderContext,
		programGL->getAttributeLocs(),
		programGL->getAttributeHash()
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
		T_ASSERT(0);
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

		if (!programGL->activateRender(m_renderContext, targetSize))
			return;

		const GLubyte* indices = reinterpret_cast< const GLubyte* >(primitives.offset * offsetMultiplier);
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
		if (!programGL->activateRender(m_renderContext, targetSize))
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

void RenderViewOpenGL::compute(IProgram* program, const int32_t* workSize)
{
#if !defined(__APPLE__)
	ProgramOpenGL* programGL = checked_type_cast< ProgramOpenGL * >(program);

	if (!programGL->activateCompute(m_renderContext))
		return;

	T_OGL_SAFE(glDispatchCompute(
		workSize[0],
		workSize[1],
		workSize[2]
	));
#endif
}

bool RenderViewOpenGL::copy(ITexture* destinationTexture, const Region& destinationRegion, ITexture* sourceTexture, const Region& sourceRegion)
{
#if !defined(__APPLE__)
	GLuint srcName = 0;
	GLenum srcTarget = GL_INVALID_ENUM;
	GLuint dstName = 0;
	GLenum dstTarget = GL_INVALID_ENUM;

	if (auto sourceRenderTarget = dynamic_type_cast< RenderTargetOpenGL* >(sourceTexture))
	{
		srcName = (GLuint)sourceRenderTarget->getInternalHandle();
		srcTarget = GL_TEXTURE_2D;
	}
	else if (auto sourceSimpleTexture = dynamic_type_cast< SimpleTextureOpenGL* >(sourceTexture))
	{
		srcName = (GLuint)sourceSimpleTexture->getInternalHandle();
		srcTarget = GL_TEXTURE_2D;
	}
	else if (auto sourceCubeTexture = dynamic_type_cast< CubeTextureOpenGL* >(sourceTexture))
	{
		srcName = (GLuint)sourceCubeTexture->getTextureName();
		srcTarget = GL_TEXTURE_CUBE_MAP;
	}
	else
		return false;

	if (auto destinationRenderTarget = dynamic_type_cast< RenderTargetOpenGL* >(destinationTexture))
	{
		dstName = (GLuint)destinationRenderTarget->getInternalHandle();
		dstTarget = GL_TEXTURE_2D;
	}
	else if (auto destinationSimpleTexture = dynamic_type_cast< SimpleTextureOpenGL* >(destinationTexture))
	{
		dstName = (GLuint)destinationSimpleTexture->getInternalHandle();
		dstTarget = GL_TEXTURE_2D;
	}
	else if (auto destinationCubeTexture = dynamic_type_cast< CubeTextureOpenGL* >(destinationTexture))
	{
		dstName = (GLuint)destinationCubeTexture->getTextureName();
		dstTarget = GL_TEXTURE_CUBE_MAP;
	}
	else
		return false;

	T_OGL_SAFE(glCopyImageSubData(
		srcName,	// srcName
		srcTarget,	// srcTarget
		sourceRegion.mip,	// srcLevel
		sourceRegion.x,	// srcX
		sourceRegion.y,	// srcY,
		sourceRegion.z,	// srcZ,
		dstName,	// dstName
		dstTarget,	// dstTarget
		destinationRegion.mip,	// dstLevel
		destinationRegion.x,	// dstX
		destinationRegion.y,	// dstY
		destinationRegion.z,	// dstZ
		sourceRegion.width,		// srcWidth
		sourceRegion.height,	// srcHeight
		1	// srcDepth
	));
	return true;
#else
	return false;
#endif
}

int32_t RenderViewOpenGL::beginTimeQuery()
{
	return 0;
}

void RenderViewOpenGL::endTimeQuery(int32_t query)
{
}

bool RenderViewOpenGL::getTimeQuery(int32_t query, bool wait, double& outDuration) const
{
	return false;
}

void RenderViewOpenGL::pushMarker(const char* const marker)
{
#if !defined(__APPLE__)
	glPushDebugGroup(
		GL_DEBUG_SOURCE_APPLICATION,
		1,
		-1,
		marker
	);
#endif
}

void RenderViewOpenGL::popMarker()
{
#if !defined(__APPLE__)
	glPopDebugGroup();
#endif
}

void RenderViewOpenGL::getStatistics(RenderViewStatistics& outStatistics) const
{
	outStatistics.drawCalls = m_drawCalls;
	outStatistics.primitiveCount = m_primitiveCount;
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
		if (!m_cursorVisible)
			SetCursor(NULL);
		else
			return false;
	}
	else
		return false;

	return true;
}

#endif

	}
}
