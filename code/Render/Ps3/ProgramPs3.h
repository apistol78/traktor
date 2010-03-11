#ifndef traktor_render_ProgramPs3_H
#define traktor_render_ProgramPs3_H

#include <map>
#include <vector>
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/IProgram.h"
#include "Render/Ps3/TypesPs3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_PS3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PoolAllocator;

	namespace render
	{

class LocalMemoryObject;
class ProgramResourcePs3;
class StateCachePs3;

class T_DLLCLASS ProgramPs3 : public IProgram
{
	T_RTTI_CLASS;

public:
	ProgramPs3();

	virtual ~ProgramPs3();

	bool create(const ProgramResourcePs3* resource);

	virtual void destroy();

	virtual void setFloatParameter(handle_t handle, float param);

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length);

	virtual void setVectorParameter(handle_t handle, const Vector4& param);

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length);

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param);

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length);

	virtual void setTextureParameter(handle_t handle, ITexture* texture);

	virtual void setStencilReference(uint32_t stencilReference);

	void bind(PoolAllocator& patchProgramPool, StateCachePs3& stateCache);

	static void unbind();

	inline const std::vector< uint8_t >& getInputSignature() const { return m_inputSignature; }

private:
	static ProgramPs3* ms_activeProgram;

	Ref< const ProgramResourcePs3 > m_resource;
	CGprogram m_vertexProgram;
	CGprogram m_pixelProgram;
	LocalMemoryObject* m_vertexShaderUCode;
	LocalMemoryObject* m_pixelShaderUCode;
	std::vector< uint8_t > m_inputSignature;
	RenderState m_renderState;
	std::vector< ProgramScalar > m_vertexScalars;
	std::vector< ProgramScalar > m_pixelScalars;
	std::vector< ProgramSampler > m_vertexSamplers;
	std::vector< ProgramSampler > m_pixelSamplers;
	std::map< handle_t, uint32_t > m_scalarParameterMap;
	std::map< handle_t, uint32_t > m_textureParameterMap;
	AlignedVector< float > m_scalarParameterData;
	RefArray< ITexture > m_textureParameterData;
	uint32_t m_patchedPixelShaderOffset;
	bool m_dirty;
};

	}
}

#endif	// traktor_render_ProgramPs3_H
