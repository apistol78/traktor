#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/ES2/RenderTargetSetOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderTargetOpenGLES2.h"
#include "Core/Heap/GcNew.h"

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

	if (desc.depthStencil)
	{
		T_OGL_SAFE(glGenRenderbuffers(1, &m_depthBuffer));
		T_OGL_SAFE(glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer));
		T_OGL_SAFE(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_width, m_height));
	}

	m_colorTextures.resize(desc.count);
	for (int i = 0; i < desc.count; ++i)
	{
		m_colorTextures[i] = gc_new< RenderTargetOpenGLES2 >(m_context);
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

ITexture* RenderTargetSetOpenGLES2::getColorTexture(int index) const
{
	return index < int(m_colorTextures.size()) ? m_colorTextures[index] : 0;
}

void RenderTargetSetOpenGLES2::swap(int index1, int index2)
{
	std::swap(m_colorTextures[index1], m_colorTextures[index2]);
}

bool RenderTargetSetOpenGLES2::read(int index, void* buffer) const
{
	return false;
}

	}
}
