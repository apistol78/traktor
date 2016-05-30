#ifndef traktor_render_ProgramOpenGL_H
#define traktor_render_ProgramOpenGL_H

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Render/IProgram.h"
#include "Render/OpenGL/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGL;
class GlslProgram;
class ProgramResource;

/*!
 * \ingroup OGL
 */
class ProgramOpenGL : public IProgram
{
	T_RTTI_CLASS;

public:
	virtual ~ProgramOpenGL();

	static Ref< ProgramResource > compile(const GlslProgram& glslProgram, int optimize, bool validate);

	static Ref< ProgramOpenGL > create(ContextOpenGL* resourceContext, const ProgramResource* resource);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setFloatParameter(handle_t handle, float param) T_OVERRIDE T_FINAL;

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length) T_OVERRIDE T_FINAL;
	
	virtual void setVectorParameter(handle_t handle, const Vector4& param) T_OVERRIDE T_FINAL;

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length) T_OVERRIDE T_FINAL;

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param) T_OVERRIDE T_FINAL;

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length) T_OVERRIDE T_FINAL;

	virtual void setTextureParameter(handle_t handle, ITexture* texture) T_OVERRIDE T_FINAL;

	virtual void setStencilReference(uint32_t stencilReference) T_OVERRIDE T_FINAL;

	bool activate(ContextOpenGL* renderContext, float targetSize[2]);

	const GLint* getAttributeLocs() const;

private:
	struct Uniform
	{
		GLint location;
		GLenum type;
		uint32_t offset;
		uint32_t length;
		bool dirty;

		Uniform()
		:	location(0)
		,	type(0)
		,	offset(0)
		,	length(0)
		,	dirty(false)
		{
		}
	};

	struct Sampler
	{
		GLint location;
		uint32_t texture;
		uint32_t stage;
		GLuint object;

		Sampler()
		:	location(0)
		,	texture(0)
		,	stage(0)
		,	object(0)
		{
		}
	};

	struct TextureSize
	{
		GLint location;
		uint32_t texture;

		TextureSize()
		:	location(0)
		,	texture(0)
		{
		}
	};

	Ref< ContextOpenGL > m_resourceContext;
	GLuint m_program;
	RenderStateOpenGL m_renderState;
	GLuint m_renderStateList;
	GLint m_locationTargetSize;
	GLint m_attributeLocs[T_OGL_MAX_USAGE_INDEX];			//!< Vertex attribute locations.
	SmallMap< handle_t, uint32_t > m_parameterMap;			//!< Parameter to data map.
	std::vector< Uniform > m_uniforms;						//!< Scalar uniforms.
	std::vector< Sampler > m_samplers;						//!< Samplers.
	std::vector< TextureSize > m_textureSize;
	AlignedVector< float > m_uniformData;					//!< Scalar uniform data.
	RefArray< ITexture > m_textures;
	float m_targetSize[2];
	bool m_textureDirty;
	bool m_validated;
	bool m_valid;
	static ProgramOpenGL* ms_activeProgram;
	
	ProgramOpenGL(ContextOpenGL* resourceContext, GLuint program, const ProgramResource* resource);
};

	}
}

#endif	// traktor_render_ProgramOpenGL_H
