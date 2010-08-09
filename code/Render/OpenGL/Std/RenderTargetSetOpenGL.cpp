#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/RenderTargetSetOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetOpenGL.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetOpenGL", RenderTargetSetOpenGL, RenderTargetSet)

RenderTargetSetOpenGL::RenderTargetSetOpenGL(IContext* resourceContext)
:	m_resourceContext(resourceContext)
,	m_width(0)
,	m_height(0)
,	m_depthBuffer(0)
{
}

RenderTargetSetOpenGL::~RenderTargetSetOpenGL()
{
	destroy();
}

bool RenderTargetSetOpenGL::create(const RenderTargetSetCreateDesc& desc)
{
	m_width = desc.width;
	m_height = desc.height;

	if (desc.depthStencil)
	{
		T_OGL_SAFE(glGenRenderbuffersEXT(1, &m_depthBuffer));
		T_OGL_SAFE(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthBuffer));
		
		if (desc.multiSample <= 1)
		{
			T_OGL_SAFE(glRenderbufferStorageEXT(
				GL_RENDERBUFFER_EXT,
				GL_DEPTH_COMPONENT24,
				m_width,
				m_height
			));
		}
		else
		{
			T_OGL_SAFE(glRenderbufferStorageMultisampleEXT(
				GL_RENDERBUFFER_EXT,
				desc.multiSample,
				GL_DEPTH_COMPONENT24,
				m_width,
				m_height
			));
		}
	}

	m_colorTextures.resize(desc.count);
	for (int i = 0; i < desc.count; ++i)
	{
		m_colorTextures[i] = new RenderTargetOpenGL(m_resourceContext);
		if (!m_colorTextures[i]->create(desc, desc.targets[i], m_depthBuffer))
			return false;
	}

	return true;
}

void RenderTargetSetOpenGL::destroy()
{
	for (RefArray< RenderTargetOpenGL >::iterator i = m_colorTextures.begin(); i != m_colorTextures.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}
	m_colorTextures.resize(0);
}

int RenderTargetSetOpenGL::getWidth() const
{
	return m_width;
}

int RenderTargetSetOpenGL::getHeight() const
{
	return m_height;
}

Ref< ITexture > RenderTargetSetOpenGL::getColorTexture(int index) const
{
	return index < m_colorTextures.size() ? m_colorTextures[index] : 0;
}

void RenderTargetSetOpenGL::swap(int index1, int index2)
{
	Ref< RenderTargetOpenGL > target1 = m_colorTextures[index1];
	Ref< RenderTargetOpenGL > target2 = m_colorTextures[index2];
	m_colorTextures[index1] = target2;
	m_colorTextures[index2] = target1;
}

bool RenderTargetSetOpenGL::read(int index, void* buffer) const
{
	return m_colorTextures[index]->read(buffer);
}

	}
}
