#ifndef traktor_render_ProgramOpenGL_H
#define traktor_render_ProgramOpenGL_H

#include <map>
#include "Core/Containers/AlignedVector.h"
#include "Render/IProgram.h"
#include "Render/OpenGL/TypesOpenGL.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class GlslProgram;
class ProgramResource;
class ContextOpenGL;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS ProgramOpenGL : public IProgram
{
	T_RTTI_CLASS;

public:
	ProgramOpenGL(ContextOpenGL* resourceContext);

	virtual ~ProgramOpenGL();

	static Ref< ProgramResource > compile(const GlslProgram& glslProgram, int optimize, bool validate);

	bool create(const ProgramResource* resource);

	virtual void destroy();

	virtual void setFloatParameter(handle_t handle, float param);

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length);
	
	virtual void setVectorParameter(handle_t handle, const Vector4& param);

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length);

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param);

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length);

	virtual void setTextureParameter(handle_t handle, ITexture* texture);

	virtual void setStencilReference(uint32_t stencilReference);

	bool activate(float targetSize[2]);

	const GLint* getAttributeLocs() const;

private:
	struct Uniform
	{
		GLint location;
		GLenum type;
		uint32_t offset;
		uint32_t length;
		bool dirty;
	};

	struct Sampler
	{
		GLint locationTexture;
		GLint locationOffset;
		uint32_t texture;
		uint32_t stage;
	};

	struct TextureData
	{
		GLenum target;
		GLuint name;
		GLint mipCount;
		float offset[4];
	};

	Ref< ContextOpenGL > m_resourceContext;
	GLhandleARB m_program;
	RenderState m_renderState;
	GLuint m_state;
	GLint m_locationTargetSize;
	GLint m_attributeLocs[T_OGL_MAX_USAGE_INDEX];		//!< Vertex attribute locations.
	std::map< handle_t, uint32_t > m_parameterMap;		//!< Parameter to data map.
	std::vector< Uniform > m_uniforms;					//!< Scalar uniforms.
	std::vector< Sampler > m_samplers;					//!< Samplers.
	AlignedVector< float > m_uniformData;				//!< Scalar uniform data.
	AlignedVector< TextureData > m_textureData;			//!< Texture data.
	float m_targetSize[2];
	GLfloat m_maxAnisotrophy;
	bool m_textureDirty;
	static ProgramOpenGL* ms_activeProgram;
};

	}
}

#endif	// traktor_render_ProgramOpenGL_H
