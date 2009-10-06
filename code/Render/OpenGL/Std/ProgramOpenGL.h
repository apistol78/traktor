#ifndef traktor_render_ProgramOpenGL_H
#define traktor_render_ProgramOpenGL_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/IProgram.h"
#include "Render/OpenGL/TypesOpenGL.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IContext;
class GlslProgram;
class ProgramResource;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS ProgramOpenGL : public IProgram
{
	T_RTTI_CLASS(ProgramOpenGL)

public:
	ProgramOpenGL(IContext* context);

	virtual ~ProgramOpenGL();

	static ProgramResource* compile(const GlslProgram& glslProgram, int optimize, bool validate);

	bool create(const ProgramResource* resource);

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
	struct Parameter
	{
		uint32_t offset;
		uint32_t length;
	};

	struct Uniform
	{
#if defined(_DEBUG)
		std::string name;
#endif
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

	Ref< IContext > m_context;

	GLhandleARB m_program;
	GLint m_attributeLocs[T_OGL_MAX_USAGE_INDEX];
	std::vector< Uniform > m_uniforms;
	std::vector< Sampler > m_samplers;
	RenderState m_renderState;
	GLuint m_state;
	std::map< handle_t, Parameter > m_parameterMap;
	AlignedVector< float > m_uniformData;
	AlignedVector< SamplerTexture > m_samplerTextures;

	static ProgramOpenGL* ms_activeProgram;
	bool m_dirty;
};

	}
}

#endif	// traktor_render_ProgramOpenGL_H
