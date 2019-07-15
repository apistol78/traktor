#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Render/IProgram.h"
#include "Render/OpenGL/Std/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

class ProgramResource;
class RenderContextOpenGL;
class ResourceContextOpenGL;

/*!
 * \ingroup OGL
 */
class ProgramOpenGL : public IProgram
{
	T_RTTI_CLASS;

public:
	virtual ~ProgramOpenGL();

	static Ref< ProgramOpenGL > create(ResourceContextOpenGL* resourceContext, const ProgramResource* resource, bool cacheEnable);

	virtual void destroy() override final;

	virtual void setFloatParameter(handle_t handle, float param) override final;

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length) override final;

	virtual void setVectorParameter(handle_t handle, const Vector4& param) override final;

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length) override final;

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param) override final;

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length) override final;

	virtual void setTextureParameter(handle_t handle, ITexture* texture) override final;

	virtual void setStructBufferParameter(handle_t handle, StructBuffer* structBuffer) override final;

	virtual void setStencilReference(uint32_t stencilReference) override final;

	bool activateRender(RenderContextOpenGL* renderContext, float targetSize[2]);

	bool activateCompute(RenderContextOpenGL* renderContext);

	const GLint* getAttributeLocs() const { return m_attributeLocs; }

	uint32_t getAttributeHash() const { return m_attributeHash; }

private:
	struct ParameterMap
	{
		uint32_t buffer;	//!< Uniform buffer index.
		uint32_t offset;	//!< Offset into uniform buffer's data.
		uint32_t size;		//!< Number of floats.

		ParameterMap()
		:	buffer(0)
		,	offset(0)
		,	size(0)
		{
		}
	};

	struct UniformBuffer
	{
		uint32_t size;
		AlignedVector< float > data;
		GLuint object;
		bool dirty;

		UniformBuffer()
		:	size(0)
		,	object(0)
		,	dirty(true)
		{
		}
	};

	struct Sampler
	{
		GLint unit;
		uint32_t state;
		uint32_t textureIndex;

		Sampler()
		:	unit(0)
		,	state(0)
		,	textureIndex(0)
		{
		}
	};

	Ref< ResourceContextOpenGL > m_resourceContext;
	GLuint m_program;
	GLuint m_renderStateList;
	uint32_t m_stencilReference;
	GLint m_attributeLocs[T_OGL_MAX_USAGE_INDEX];			//!< Vertex attribute locations.
	uint32_t m_attributeHash;
	UniformBuffer m_uniformBuffers[3];
	RefArray< ITexture > m_textures;
	AlignedVector< Sampler > m_samplers;
	SmallMap< handle_t, ParameterMap > m_parameterMap;

	// SmallMap< handle_t, uint32_t > m_parameterMap;			//!< Parameter to data map.
	// AlignedVector< Uniform > m_uniforms;					//!< Scalar uniforms.
	// AlignedVector< Sampler > m_samplers;					//!< Samplers.
	// AlignedVector< TextureSize > m_textureSize;
	// AlignedVector< float > m_uniformData;					//!< Scalar uniform data.
	// RefArray< ITexture > m_textures;
	// float m_targetSize[2];
	// bool m_textureDirty;
	// bool m_validated;
	// bool m_valid;

	ProgramOpenGL(ResourceContextOpenGL* resourceContext, GLuint program, const ProgramResource* resource);
};

	}
}
