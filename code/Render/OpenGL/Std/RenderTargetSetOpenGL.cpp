#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/RenderTargetSetOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetOpenGL.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetOpenGL", RenderTargetSetOpenGL, RenderTargetSet)

RenderTargetSetOpenGL::RenderTargetSetOpenGL(IContext* resourceContext, BlitHelper* blitHelper)
:	m_resourceContext(resourceContext)
,	m_blitHelper(blitHelper)
,	m_width(0)
,	m_height(0)
,	m_targetWidth(0)
,	m_targetHeight(0)
,	m_depthBuffer(0)
{
}

RenderTargetSetOpenGL::~RenderTargetSetOpenGL()
{
	destroy();
}

bool RenderTargetSetOpenGL::create(const RenderTargetSetCreateDesc& desc, bool backBuffer)
{
	if (!opengl_have_extension(E_GL_EXT_framebuffer_object))
	{
		log::error << L"Cannot create FBO; not supported by OpenGL driver" << Endl;
		return false;
	}

	// When using MSAA we require both MSAA framebuffer extension and framebuffer blitting.
	if (
		desc.multiSample > 1 &&
		!(opengl_have_extension(E_GL_EXT_framebuffer_multisample) && opengl_have_extension(E_GL_EXT_framebuffer_blit))
	)
	{
		log::error << L"Cannot create multisample FBO; not supported by OpenGL driver" << Endl;
		return false;
	}

	m_width = desc.width;
	m_height = desc.height;
	m_targetWidth = desc.width;
	m_targetHeight = desc.height;

	if (!opengl_have_extension(E_T_rendertarget_non_power_of_two))
	{
		if (!isLog2(m_width) || !isLog2(m_height))
		{
			log::warning << L"Trying to create non-power-of-2 render target but OpenGL doesn't support NPO2; scaling applied" << Endl;
			m_width = nearestLog2(m_width);
			m_height = nearestLog2(m_height);
		}
		
		// Only backbuffer allowed to be partially rendered; if expected
		// to be used as texture then we need to ensure everything is renderered.
		if (!backBuffer)
		{
			m_targetWidth = m_width;
			m_targetHeight = m_height;
		}
	}

	if (desc.createDepthStencil)
	{
		T_OGL_SAFE(glGenRenderbuffersEXT(1, &m_depthBuffer));
		T_OGL_SAFE(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthBuffer));
		
		if (desc.multiSample <= 1)
		{
			T_OGL_SAFE(glRenderbufferStorageEXT(
				GL_RENDERBUFFER_EXT,
				GL_DEPTH_STENCIL_EXT,
				m_width,
				m_height
			));
		}
		else
		{
			T_OGL_SAFE(glRenderbufferStorageMultisampleEXT(
				GL_RENDERBUFFER_EXT,
				desc.multiSample,
				GL_DEPTH_STENCIL_EXT,
				m_width,
				m_height
			));
		}
	}

	m_colorTextures.resize(desc.count);
	for (int i = 0; i < desc.count; ++i)
	{
		m_colorTextures[i] = new RenderTargetOpenGL(m_resourceContext, m_blitHelper);
		if (!m_colorTextures[i]->create(desc, desc.targets[i], m_depthBuffer, backBuffer))
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
	return m_targetWidth;
}

int RenderTargetSetOpenGL::getHeight() const
{
	return m_targetHeight;
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
