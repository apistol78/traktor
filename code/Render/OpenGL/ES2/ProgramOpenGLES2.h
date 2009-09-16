#ifndef traktor_render_ProgramOpenGLES2_H
#define traktor_render_ProgramOpenGLES2_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/IProgram.h"
#include "Render/OpenGL/TypesOpenGL.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_ES2_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class GlslProgram;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS ProgramOpenGLES2 : public IProgram
{
	T_RTTI_CLASS(ProgramOpenGLES2)

public:
	ProgramOpenGLES2();

	virtual ~ProgramOpenGLES2();

	bool create(const GlslProgram& glslProgram);

	virtual void destroy();

	virtual void setFloatParameter(handle_t handle, float param);

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length);
	
	virtual void setVectorParameter(handle_t handle, const Vector4& param);

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length);

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param);

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length);

	virtual void setSamplerTexture(handle_t handle, ITexture* texture);

	virtual void setStencilReference(uint32_t stencilReference);

	virtual bool isOpaque() const;

	bool activate();

	const GLint* getAttributeLocs() const;

private:
	struct Uniform
	{
		GLint location;
		GLenum type;
		uint32_t offset;
		uint32_t length;
	};

	struct Sampler
	{
		GLint location;
		GLint locationOriginScale;
		uint32_t texture;
		uint32_t unit;
	};

	struct SamplerTexture
	{
		GLenum target;
		GLuint name;
		Vector4 originScale;
		uint32_t mipCount;
	};

	GLuint m_program;
	GLint m_attributeLocs[T_OGL_MAX_USAGE_INDEX];
	std::vector< Uniform > m_uniforms;
	std::vector< Sampler > m_samplers;
	RenderState m_renderState;
	std::map< handle_t, uint32_t > m_parameterMap;
	AlignedVector< float > m_uniformData;
	std::vector< bool > m_uniformDataDirty;
	AlignedVector< SamplerTexture > m_samplerTextures;

	static ProgramOpenGLES2* ms_activeProgram;
	bool m_dirty;
};

	}
}

#endif	// traktor_render_ProgramOpenGLES2_H
