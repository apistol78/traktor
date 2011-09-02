#include <cstring>
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/IContext.h"
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
	GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB
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

	if (!convertTextureFormat(desc.format, m_pixelSize, m_components, m_format, m_type))
		return false;

	T_OGL_SAFE(glGenTextures(1, &m_textureName));

	m_data.resize(m_side * m_side * m_pixelSize);

	if (desc.immutable)
	{
		T_OGL_SAFE(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		T_OGL_SAFE(glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, m_textureName));

		for (int face = 0; face < 6; ++face)
		{
			for (int i = 0; i < desc.mipCount; ++i)
			{
				uint32_t side = m_side >> i;

				T_ASSERT (desc.initialData[face * desc.mipCount + i].pitch >= side * m_pixelSize);
				const uint8_t* s = static_cast< const uint8_t* >(desc.initialData[face * desc.mipCount + i].data);
				uint8_t* d = static_cast< uint8_t* >(&m_data[0]);
				
				for (int y = 0; y < m_side; ++y)
				{
					std::memcpy(d, s, side * m_pixelSize);
					s += desc.initialData[face * desc.mipCount + i].pitch;
					d += side * m_pixelSize;
				}

				T_OGL_SAFE(glTexImage2D(
					c_cubeFaces[face],
					i,
					m_components,
					side,
					side,
					0,
					m_format,
					m_type,
					&m_data[0]
				));
			}
		}
	}

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
	lock.bits = &m_data[0];
	return true;
}

void CubeTextureOpenGL::unlock(int side, int level)
{
}

void CubeTextureOpenGL::bind(GLuint unit, const SamplerState& samplerState, GLint locationTexture)
{
	T_FATAL_ERROR;
}

	}
}
