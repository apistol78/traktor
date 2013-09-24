#include <cstring>
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"
#include "Render/OpenGL/Std/CubeTextureOpenGL.h"
#include "Render/OpenGL/Std/UtilitiesOpenGL.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

#if !defined(__APPLE__)
const GLenum c_cubeFaces[] =
{
	GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT
};
#else
const GLenum c_cubeFaces[] =
{
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};
#endif

struct DeleteTextureCallback : public IContext::IDeleteCallback
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CubeTextureOpenGL", CubeTextureOpenGL, ICubeTexture)

CubeTextureOpenGL::CubeTextureOpenGL(IContext* resourceContext)
:	m_resourceContext(resourceContext)
,	m_textureName(0)
,	m_side(0)
,	m_pixelSize(0)
,	m_mipCount(0)
,	m_components(0)
,	m_format(0)
,	m_type(0)
{
}

CubeTextureOpenGL::~CubeTextureOpenGL()
{
	destroy();
}

bool CubeTextureOpenGL::create(const CubeTextureCreateDesc& desc)
{
	m_side = desc.side;

	if (desc.sRGB)
	{
		if (!convertTextureFormat_sRGB(desc.format, m_pixelSize, m_components, m_format, m_type))
			return false;
	}
	else
	{
		if (!convertTextureFormat(desc.format, m_pixelSize, m_components, m_format, m_type))
			return false;
	}

	T_OGL_SAFE(glGenTextures(1, &m_textureName));

	m_data.reset(new uint8_t [m_side * m_side * m_pixelSize]);

	if (desc.immutable)
	{
		T_OGL_SAFE(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		T_OGL_SAFE(glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureName));

		for (int face = 0; face < 6; ++face)
		{
			for (int i = 0; i < desc.mipCount; ++i)
			{
				uint32_t side = getTextureMipSize(m_side, i);
				if (desc.format >= TfDXT1 && desc.format <= TfDXT5)
				{
					uint32_t mipPitch = getTextureMipPitch(desc.format, side, side);
					T_OGL_SAFE(glCompressedTexImage2D(
						c_cubeFaces[face],
						i,
						m_components,
						side,
						side,
						0,
						mipPitch,
						desc.initialData[face * desc.mipCount + i].data
					));
				}
				else
				{
					T_OGL_SAFE(glTexImage2D(
						c_cubeFaces[face],
						i,
						m_components,
						side,
						side,
						0,
						m_format,
						m_type,
						desc.initialData[face * desc.mipCount + i].data
					));
				}
			}
		}
	}

	m_mipCount = desc.mipCount;
	return true;
}

void CubeTextureOpenGL::destroy()
{
	if (m_textureName)
	{
		m_resourceContext->deleteResource(new DeleteTextureCallback(m_textureName));
		m_textureName = 0;
	}
}

ITexture* CubeTextureOpenGL::resolve()
{
	return this;
}

int CubeTextureOpenGL::getWidth() const
{
	return m_side;
}

int CubeTextureOpenGL::getHeight() const
{
	return m_side;
}

int CubeTextureOpenGL::getDepth() const
{
	return 1;
}

bool CubeTextureOpenGL::lock(int side, int level, Lock& lock)
{
	lock.pitch = (m_side >> level) * m_pixelSize;
	lock.bits = m_data.ptr();
	return true;
}

void CubeTextureOpenGL::unlock(int side, int level)
{
}

void CubeTextureOpenGL::bindTexture(ContextOpenGL* renderContext, uint32_t samplerObject)
{
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureName));
	renderContext->bindSamplerStateObject(GL_TEXTURE_CUBE_MAP, samplerObject, m_mipCount > 1, 0.0f);
}

void CubeTextureOpenGL::bindSize(GLint locationSize)
{
	T_OGL_SAFE(glUniform4f(locationSize, GLfloat(m_side), GLfloat(m_side), GLfloat(m_side), GLfloat(1.0f)));
}

	}
}
