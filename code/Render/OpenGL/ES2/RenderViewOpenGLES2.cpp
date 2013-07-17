#include "Core/Misc/SafeDestroy.h"
#include "Render/OpenGL/ES2/ContextOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderViewOpenGLES2.h"

#if !defined(T_OFFLINE_ONLY)

#	include "Core/Log/Log.h"
#	include "Render/OpenGL/ES2/VertexBufferOpenGLES2.h"
#	include "Render/OpenGL/ES2/IndexBufferOpenGLES2.h"
#	include "Render/OpenGL/ES2/ProgramOpenGLES2.h"
#	include "Render/OpenGL/ES2/RenderSystemOpenGLES2.h"
#	include "Render/OpenGL/ES2/RenderTargetOpenGLES2.h"
#	include "Render/OpenGL/ES2/RenderTargetSetOpenGLES2.h"
#	include "Render/OpenGL/ES2/StateCache.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewOpenGLES2", RenderViewOpenGLES2, IRenderView)

RenderViewOpenGLES2::RenderViewOpenGLES2(
	ContextOpenGLES2* globalContext,
	ContextOpenGLES2* context
)
:	m_globalContext(globalContext)
,	m_context(context)
,	m_stateCache(new StateCache())
{
	m_viewport = Viewport(
		0,
		0,
		getWidth(),
		getHeight(),
		0.0f,
		1.0f
	);
}

RenderViewOpenGLES2::~RenderViewOpenGLES2()
{
}

bool RenderViewOpenGLES2::nextEvent(RenderEvent& outEvent)
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

#endif

	return false;
}

void RenderViewOpenGLES2::close()
{
	m_context = 0;
	m_globalContext = 0;
}

bool RenderViewOpenGLES2::reset(const RenderViewDefaultDesc& desc)
{
	return false;
}

bool RenderViewOpenGLES2::reset(int32_t width, int32_t height)
{
	if (!m_context->getLandscape())
		m_context->resize(width, height);
	else
		m_context->resize(height, width);
	return true;
}

int RenderViewOpenGLES2::getWidth() const
{
	if (!m_context->getLandscape())
		return m_context->getWidth();
	else
		return m_context->getHeight();
}

int RenderViewOpenGLES2::getHeight() const
{
	if (!m_context->getLandscape())
		return m_context->getHeight();
	else
		return m_context->getWidth();
}

bool RenderViewOpenGLES2::isActive() const
{
	return true;
}

bool RenderViewOpenGLES2::isFullScreen() const
{
#if defined(T_OPENGL_ES2_HAVE_EGL)
	return false;
#else
	return true;
#endif
}

void RenderViewOpenGLES2::showCursor()
{
}

void RenderViewOpenGLES2::hideCursor()
{
}

bool RenderViewOpenGLES2::setGamma(float gamma)
{
	return false;
}

void RenderViewOpenGLES2::setViewport(const Viewport& viewport)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_context);

	if (m_renderTargetStack.empty())
		m_viewport = viewport;
	else
		m_renderTargetStack.top().viewport = viewport;

	if (m_renderTargetStack.empty() && m_context->getLandscape())
	{
		T_OGL_SAFE(glViewport(
			viewport.top,
			viewport.left,
			viewport.height,
			viewport.width
		));
	}
	else
	{
		T_OGL_SAFE(glViewport(
			viewport.left,
			viewport.top,
			viewport.width,
			viewport.height
		));
	}

	T_OGL_SAFE(glDepthRangef(
		viewport.nearZ,
		viewport.farZ
	));
}

Viewport RenderViewOpenGLES2::getViewport()
{
	if (m_renderTargetStack.empty())
		return m_viewport;
	else
		return m_renderTargetStack.top().viewport;
}

SystemWindow RenderViewOpenGLES2::getSystemWindow()
{
	return SystemWindow();
}

bool RenderViewOpenGLES2::begin(EyeType eye)
{
#if !TARGET_OS_IPHONE
	if (!m_globalContext->lock().wait())
		return false;
#endif

	if (!m_context->enter())
		return false;
		
	m_context->bindPrimary();
	
	if (m_context->getLandscape())
	{
		T_OGL_SAFE(glViewport(
			m_viewport.top,
			m_viewport.left,
			m_viewport.height,
			m_viewport.width
		));
	}
	else
	{
		T_OGL_SAFE(glViewport(
			m_viewport.left,
			m_viewport.top,
			m_viewport.width,
			m_viewport.height
		));
	}

	T_OGL_SAFE(glDepthRangef(
		m_viewport.nearZ,
		m_viewport.farZ
	));

	//glGetFramebufferAttachmentParameteriv(
	//	GL_FRAMEBUFFER, 
	//	GL_DEPTH_ATTACHMENT,
	//	GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
	//	&m_primaryDepth
	//);

	return true;
}

bool RenderViewOpenGLES2::begin(RenderTargetSet* renderTargetSet)
{
	T_FATAL_ERROR;
	return false;
}

bool RenderViewOpenGLES2::begin(RenderTargetSet* renderTargetSet, int renderTarget)
{
	RenderTargetSetOpenGLES2* rts = checked_type_cast< RenderTargetSetOpenGLES2* >(renderTargetSet);

	if (!rts->bind(/*m_primaryDepth*/0, renderTarget))
		return false;
	
	RenderTargetStack s;
	s.renderTargetSet = rts;
	s.renderTarget = renderTarget;
	s.viewport = Viewport(0, 0, rts->getWidth(), rts->getHeight(), 0.0f, 1.0f);
	
	T_OGL_SAFE(glViewport(
		s.viewport.left,
		s.viewport.top,
		s.viewport.width,
		s.viewport.height
	));

	T_OGL_SAFE(glDepthRangef(
		s.viewport.nearZ,
		s.viewport.farZ
	));

	m_renderTargetStack.push(s);
	return true;
}

void RenderViewOpenGLES2::clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil)
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
	if (!cm)
		return;

	if (cm & GL_COLOR_BUFFER_BIT)
	{
		m_stateCache->setColorMask(RenderStateOpenGL::CmAll);
		float r = colors[0].getRed();
		float g = colors[0].getGreen();
		float b = colors[0].getBlue();
		float a = colors[0].getAlpha();
		T_OGL_SAFE(glClearColor(r, g, b, a));
	}

	if (cm & GL_DEPTH_BUFFER_BIT)
	{
		m_stateCache->setDepthMask(GL_TRUE);
		T_OGL_SAFE(glClearDepthf(depth));
	}

	if (cm & GL_STENCIL_BUFFER_BIT)
		T_OGL_SAFE(glClearStencil(stencil));

	T_OGL_SAFE(glClear(cm));
}

void RenderViewOpenGLES2::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	VertexBufferOpenGLES2* vertexBufferGL = checked_type_cast< VertexBufferOpenGLES2* >(vertexBuffer);
	IndexBufferOpenGLES2* indexBufferGL = checked_type_cast< IndexBufferOpenGLES2* >(indexBuffer);
	ProgramOpenGLES2* programGL = checked_type_cast< ProgramOpenGLES2 * >(program);

	vertexBufferGL->activate(m_stateCache);

	float targetSize[2];
	float postTransform[4];
	bool invertCull;

	if (m_renderTargetStack.empty())
	{
		targetSize[0] = float(getWidth());
		targetSize[1] = float(getHeight());
		
		if (m_context->getLandscape())
		{
			postTransform[0] = 0.0f;
			postTransform[1] = -1.0f;
			postTransform[2] = 1.0f;
			postTransform[3] = 0.0f;
		}
		else
		{
			postTransform[0] = 1.0f;
			postTransform[1] = 0.0f;
			postTransform[2] = 0.0f;
			postTransform[3] = 1.0f;
		}
		
		invertCull = false;
	}
	else
	{
		const RenderTargetSetOpenGLES2* rts = m_renderTargetStack.top().renderTargetSet;
		targetSize[0] = float(rts->getWidth());
		targetSize[1] = float(rts->getHeight());

		postTransform[0] = 1.0f;
		postTransform[1] = 0.0f;
		postTransform[2] = 0.0f;
		postTransform[3] = -1.0f;
		
		invertCull = true;
	}

	if (!programGL->activate(m_stateCache, targetSize, postTransform, invertCull, 0))
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
		T_ASSERT (0);
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
}

void RenderViewOpenGLES2::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	VertexBufferOpenGLES2* vertexBufferGL = checked_type_cast< VertexBufferOpenGLES2* >(vertexBuffer);
	IndexBufferOpenGLES2* indexBufferGL = checked_type_cast< IndexBufferOpenGLES2* >(indexBuffer);
	ProgramOpenGLES2* programGL = checked_type_cast< ProgramOpenGLES2 * >(program);

	vertexBufferGL->activate(m_stateCache);

	float targetSize[2];
	float postTransform[4];
	bool invertCull;

	if (m_renderTargetStack.empty())
	{
		targetSize[0] = float(getWidth());
		targetSize[1] = float(getHeight());
		
		if (m_context->getLandscape())
		{
			postTransform[0] = 0.0f;
			postTransform[1] = -1.0f;
			postTransform[2] = 1.0f;
			postTransform[3] = 0.0f;
		}
		else
		{
			postTransform[0] = 1.0f;
			postTransform[1] = 0.0f;
			postTransform[2] = 0.0f;
			postTransform[3] = 1.0f;
		}
		
		invertCull = false;
	}
	else
	{
		const RenderTargetSetOpenGLES2* rts = m_renderTargetStack.top().renderTargetSet;
		targetSize[0] = float(rts->getWidth());
		targetSize[1] = float(rts->getHeight());

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
		T_ASSERT (0);
	}

	for (uint32_t i = 0; i < instanceCount; ++i)
	{
		if (!programGL->activate(m_stateCache, targetSize, postTransform, invertCull, i))
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
	}
}

void RenderViewOpenGLES2::end()
{
	if (m_renderTargetStack.empty())
		return;

	m_renderTargetStack.top().renderTargetSet->setContentValid(true);
	m_renderTargetStack.pop();
	
	if (!m_renderTargetStack.empty())
	{
		RenderTargetStack& s = m_renderTargetStack.top();
		
		s.renderTargetSet->bind(/*m_primaryTargetSet->getDepthBuffer()*/0, s.renderTarget);
		
		T_OGL_SAFE(glViewport(
			s.viewport.left,
			s.viewport.top,
			s.viewport.width,
			s.viewport.height
		));

		T_OGL_SAFE(glDepthRangef(
			s.viewport.nearZ,
			s.viewport.farZ
		));
	}
	else
	{
		m_context->bindPrimary();
		
		if (m_context->getLandscape())
		{
			T_OGL_SAFE(glViewport(
				m_viewport.top,
				m_viewport.left,
				m_viewport.height,
				m_viewport.width
			));
		}
		else
		{
			T_OGL_SAFE(glViewport(
				m_viewport.left,
				m_viewport.top,
				m_viewport.width,
				m_viewport.height
			));
		}

		T_OGL_SAFE(glDepthRangef(
			m_viewport.nearZ,
			m_viewport.farZ
		));
	}
}

void RenderViewOpenGLES2::present()
{	
	m_context->swapBuffers();
	m_context->leave();

#if !TARGET_OS_IPHONE
	m_globalContext->lock().release();
#endif
	m_globalContext->deleteResources();
}

void RenderViewOpenGLES2::pushMarker(const char* const marker)
{
}

void RenderViewOpenGLES2::popMarker()
{
}

void RenderViewOpenGLES2::getStatistics(RenderViewStatistics& outStatistics) const
{
}

bool RenderViewOpenGLES2::getBackBufferContent(void* buffer) const
{
	return false;
}

	}
}

#endif
