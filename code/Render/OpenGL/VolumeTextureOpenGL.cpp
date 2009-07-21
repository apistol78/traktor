#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/VolumeTextureOpenGL.h"
#include "Render/OpenGL/ContextOpenGL.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteTextureCallback : public ContextOpenGL::DeleteCallback
{
	GLuint m_textureName;

	DeleteTextureCallback(GLuint textureName)
	:	m_textureName(textureName)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteTextures(1, &m_textureName));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VolumeTextureOpenGL", VolumeTextureOpenGL, IVolumeTexture)

VolumeTextureOpenGL::VolumeTextureOpenGL(ContextOpenGL* context)
:	m_context(context)
,	m_textureName(0)
,	m_width(0)
,	m_height(0)
,	m_depth(0)
{
}

bool VolumeTextureOpenGL::create(const VolumeTextureCreateDesc& desc)
{
	return false;
}

void VolumeTextureOpenGL::destroy()
{
	if (m_textureName)
	{
		m_context->deleteResource(new DeleteTextureCallback(m_textureName));
		m_textureName = 0;
	}
}

int VolumeTextureOpenGL::getWidth() const
{
	return m_width;
}

int VolumeTextureOpenGL::getHeight() const
{
	return m_height;
}

int VolumeTextureOpenGL::getDepth() const
{
	return m_depth;
}

	}
}
