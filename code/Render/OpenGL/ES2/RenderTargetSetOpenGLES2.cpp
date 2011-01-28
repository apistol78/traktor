#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/ES2/RenderTargetSetOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderTargetOpenGLES2.h"

#if !defined(T_OFFLINE_ONLY)

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetOpenGLES2", RenderTargetSetOpenGLES2, RenderTargetSet)

RenderTargetSetOpenGLES2::RenderTargetSetOpenGLES2(IContext* context)
:	m_context(context)
,	m_width(0)
,	m_height(0)
,	m_depthBuffer(0)
,	m_clearMask(0)
{
}

RenderTargetSetOpenGLES2::~RenderTargetSetOpenGLES2()
{
	destroy();
}

bool RenderTargetSetOpenGLES2::create(const RenderTargetSetCreateDesc& desc)
{
	m_width = desc.width;
	m_height = desc.height;
	m_clearMask = GL_COLOR_BUFFER_BIT;

	if (desc.createDepthStencil)
	{
		T_OGL_SAFE(glGenRenderbuffers(1, &m_depthBuffer));
		T_OGL_SAFE(glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer));
		if (!desc.ignoreStencil)
		{
			T_OGL_SAFE(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, m_width, m_height));
			m_clearMask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
		}
		else
		{
			T_OGL_SAFE(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_width, m_height));
			m_clearMask |= GL_DEPTH_BUFFER_BIT;
		}
	}

	m_colorTextures.resize(desc.count);
	for (int i = 0; i < desc.count; ++i)
	{
		m_colorTextures[i] = new RenderTargetOpenGLES2(m_context);
		if (!m_colorTextures[i]->create(desc, desc.targets[i], m_depthBuffer))
			return false;
	}

	return true;
}

void RenderTargetSetOpenGLES2::destroy()
{
	for (RefArray< RenderTargetOpenGLES2 >::iterator i = m_colorTextures.begin(); i != m_colorTextures.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}
	m_colorTextures.resize(0);
}

int RenderTargetSetOpenGLES2::getWidth() const
{
	return m_width;
}

int RenderTargetSetOpenGLES2::getHeight() const
{
	return m_height;
}

Ref< ITexture > RenderTargetSetOpenGLES2::getColorTexture(int index) const
{
	return index < int(m_colorTextures.size()) ? m_colorTextures[index] : 0;
}

void RenderTargetSetOpenGLES2::swap(int index1, int index2)
{
	Ref< RenderTargetOpenGLES2 > target1 = m_colorTextures[index1];
	Ref< RenderTargetOpenGLES2 > target2 = m_colorTextures[index2];
	m_colorTextures[index1] = target2;
	m_colorTextures[index2] = target1;
}

bool RenderTargetSetOpenGLES2::read(int index, void* buffer) const
{
	return false;
}

	}
}

#endif
