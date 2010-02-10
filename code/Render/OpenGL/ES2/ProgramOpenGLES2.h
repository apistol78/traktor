#ifndef traktor_render_ProgramOpenGLES2_H
#define traktor_render_ProgramOpenGLES2_H

#include <map>
#include "Core/Containers/AlignedVector.h"
#include "Render/IProgram.h"
#include "Render/OpenGL/TypesOpenGL.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_ES2_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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
class T_DLLCLASS ProgramOpenGLES2 : public IProgram
{
	T_RTTI_CLASS;

public:
	ProgramOpenGLES2(IContext* context);

	virtual ~ProgramOpenGLES2();

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

	bool activate(bool landspace);

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
		uint32_t texture;
		uint32_t stage;
	};

	struct TextureData
	{
		GLenum target;
		GLuint name;
	};

	Ref< IContext > m_context;
	GLuint m_program;
	RenderState m_renderState;
	
	GLint m_postOrientationCoeffs;
	GLint m_attributeLocs[T_OGL_MAX_USAGE_INDEX];

	std::map< handle_t, uint32_t > m_parameterMap;

	std::vector< Uniform > m_uniforms;
	std::vector< Sampler > m_samplers;
	
	AlignedVector< float > m_uniformData;
	std::vector< bool > m_uniformDataDirty;

	AlignedVector< TextureData > m_textureData;

	static ProgramOpenGLES2* ms_activeProgram;
	bool m_dirty;

	bool createFromSource(const ProgramResource* resource);

	bool createFromBinary(const ProgramResource* resource);
};

	}
}

#endif	// traktor_render_ProgramOpenGLES2_H
