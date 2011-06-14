#include "Core/Misc/SafeDestroy.h"
#include "Render/OpenGL/ES2/BlitHelper.h"
#include "Render/OpenGL/ES2/ContextOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderViewOpenGLES2.h"

#if !defined(T_OFFLINE_ONLY)

#	include "Render/OpenGL/ES2/RenderSystemOpenGLES2.h"
#	include "Render/OpenGL/ES2/VertexBufferOpenGLES2.h"
#	include "Render/OpenGL/ES2/IndexBufferOpenGLES2.h"
#	include "Render/OpenGL/ES2/ProgramOpenGLES2.h"
#	include "Render/OpenGL/ES2/RenderTargetSetOpenGLES2.h"
#	include "Render/OpenGL/ES2/RenderTargetOpenGLES2.h"
#	include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewOpenGLES2", RenderViewOpenGLES2, IRenderView)

RenderViewOpenGLES2::RenderViewOpenGLES2(
	ContextOpenGLES2* globalContext,
	ContextOpenGLES2* context,
	BlitHelper* blitHelper
)
:	m_globalContext(globalContext)
,	m_context(context)
,	m_blitHelper(blitHelper)
,	m_currentDirty(true)
{
	updatePrimaryTarget();
}

RenderViewOpenGLES2::~RenderViewOpenGLES2()
{
}

void RenderViewOpenGLES2::close()
{
	m_context = 0;
	m_globalContext = 0;
	m_blitHelper = 0;
}

bool RenderViewOpenGLES2::reset(const RenderViewDefaultDesc& desc)
{
	return false;
}

void RenderViewOpenGLES2::resize(int32_t width, int32_t height)
{
	if (!m_context->getLandscape())
		m_context->resize(width, height);
	else
		m_context->resize(height, width);

	updatePrimaryTarget();
}

int RenderViewOpenGLES2::getWidth() const
{
	return m_primaryTargetSet->getWidth();
}

int RenderViewOpenGLES2::getHeight() const
{
	return m_primaryTargetSet->getHeight();
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

bool RenderViewOpenGLES2::setGamma(float gamma)
{
	return false;
}

void RenderViewOpenGLES2::setViewport(const Viewport& viewport)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_context);

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

Viewport RenderViewOpenGLES2::getViewport()
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_context);

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

bool RenderViewOpenGLES2::begin(EyeType eye)
{
#if !TARGET_OS_IPHONE
	if (!m_globalContext->lock().wait())
		return false;
#endif

	if (!m_context->enter())
		return false;

	return begin(m_primaryTargetSet, 0);
}

bool RenderViewOpenGLES2::begin(RenderTargetSet* renderTargetSet, int renderTarget)
{
	RenderTargetSetOpenGLES2* rts = checked_type_cast< RenderTargetSetOpenGLES2* >(renderTargetSet);
	RenderTargetOpenGLES2* rt = checked_type_cast< RenderTargetOpenGLES2* >(rts->getColorTexture(renderTarget));
	
	rt->bind(m_primaryTargetSet->getDepthBuffer());
	rt->enter();

	RenderTargetScope scope;
	scope.renderTargetSet = rts;
	scope.renderTarget = rt;
	m_renderTargetStack.push(scope);

	rts->setContentValid(true);
	m_currentDirty = true;

	return true;
}

void RenderViewOpenGLES2::clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil)
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
	
	if (!m_renderTargetStack.empty())
		cm &= m_renderTargetStack.top().renderTargetSet->getClearMask();
	else
		cm &= ~GL_STENCIL_BUFFER_BIT;
	
	if (!cm)
		return;

	if (cm & GL_COLOR_BUFFER_BIT)
	{
		m_context->setColorMask(RenderState::CmAll);
		float r = color[0];
		float g = color[1];
		float b = color[2];
		float a = color[3];
		T_OGL_SAFE(glClearColor(r, g, b, a));
	}

	if (cm & GL_DEPTH_BUFFER_BIT)
	{
		m_context->setDepthMask(GL_TRUE);
		T_OGL_SAFE(glClearDepthf(depth));
	}

	if (cm & GL_STENCIL_BUFFER_BIT)
		T_OGL_SAFE(glClearStencil(stencil));

	T_OGL_SAFE(glClear(cm));
}

void RenderViewOpenGLES2::setVertexBuffer(VertexBuffer* vertexBuffer)
{
	VertexBufferOpenGLES2* vb = checked_type_cast< VertexBufferOpenGLES2* >(vertexBuffer);
	//if (vb != m_currentVertexBuffer)
	{
		m_currentVertexBuffer = vb;
		m_currentDirty = true;
	}
}

void RenderViewOpenGLES2::setIndexBuffer(IndexBuffer* indexBuffer)
{
	IndexBufferOpenGLES2* ib = checked_type_cast< IndexBufferOpenGLES2* >(indexBuffer);
	//if (ib != m_currentIndexBuffer)
	{
		m_currentIndexBuffer = ib;
		m_currentDirty = true;
	}
}

void RenderViewOpenGLES2::setProgram(IProgram* program)
{
	ProgramOpenGLES2* p = checked_type_cast< ProgramOpenGLES2 * >(program);
	//if (p != m_currentProgram)
	{
		m_currentProgram = p;
		m_currentDirty = true;
	}
}

void RenderViewOpenGLES2::draw(const Primitives& primitives)
{
	if (m_currentDirty)
	{
		if (!m_currentProgram || !m_currentVertexBuffer)
			return;

		const RenderTargetOpenGLES2* rt = m_renderTargetStack.top().renderTarget;

		float targetSize[2];
		targetSize[0] = float(rt->getWidth());
		targetSize[1] = float(rt->getHeight());

		if (!m_currentProgram->activate(targetSize))
			return;

		m_currentVertexBuffer->activate(
			m_currentProgram->getAttributeLocs()
		);

		m_currentDirty = false;
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

	if (primitives.indexed)
	{
		T_ASSERT_M (m_currentIndexBuffer, L"No index buffer");

		GLenum indexType;
		GLint offsetMultiplier;

		switch (m_currentIndexBuffer->getIndexType())
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

		m_currentIndexBuffer->bind();

		const GLubyte* indices = static_cast< const GLubyte* >(m_currentIndexBuffer->getIndexData()) + primitives.offset * offsetMultiplier;

		T_OGL_SAFE(glDrawElements(
			primitiveType,
			vertexCount,
			indexType,
			indices
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

void RenderViewOpenGLES2::end()
{
	T_ASSERT (!m_renderTargetStack.empty());
	
	m_renderTargetStack.pop();
	
	if (!m_renderTargetStack.empty())
	{
		m_renderTargetStack.top().renderTarget->bind(
			m_primaryTargetSet->getDepthBuffer()
		);
		m_renderTargetStack.top().renderTarget->enter();
	}
}

void RenderViewOpenGLES2::present()
{
	RenderTargetOpenGLES2* rt = checked_type_cast< RenderTargetOpenGLES2* >(m_primaryTargetSet->getColorTexture(0));

	// Blit primary buffer to frame.
	m_context->bindPrimary();
	
	T_OGL_SAFE(glViewport(
		0,
		0,
		m_context->getWidth(),
		m_context->getHeight()
	));
	
	m_context->setColorMask(RenderState::CmAll);
	T_OGL_SAFE(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));

	m_blitHelper->blit(rt->getColorTexture());
		
	// Swap frames.
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

bool RenderViewOpenGLES2::updatePrimaryTarget()
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_globalContext);
	
	RenderTargetSetCreateDesc desc;
	
	desc.count = 1;
	desc.multiSample = 0;
	desc.createDepthStencil = true;
	
	if (!m_context->getLandscape())
	{
		desc.width = m_context->getWidth();
		desc.height = m_context->getHeight();
	}
	else
	{
		desc.width = m_context->getHeight();
		desc.height = m_context->getWidth();
	}
	
	// Create primary target less than full resolution
	// in order to reduce bandwidth.
	const int32_t c_resolutionDenom = 1;
	
	desc.width /= c_resolutionDenom;
	desc.height /= c_resolutionDenom;
	
	desc.targets[0].format = TfR8G8B8A8;
	
	safeDestroy(m_primaryTargetSet);
	
	m_primaryTargetSet = new RenderTargetSetOpenGLES2(m_context);
	m_primaryTargetSet->create(desc);

	return true;
}

	}
}

#endif
