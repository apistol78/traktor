#ifndef traktor_render_ProgramSw_H
#define traktor_render_ProgramSw_H

#include <map>
#include "Render/IProgram.h"
#include "Render/Sw/Core/Types.h"
#include "Render/Sw/Core/IntrProgram.h"
#include "Render/Sw/Core/Processor.h"
#include "Render/Sw/RenderStateDesc.h"

namespace traktor
{
	namespace render
	{

class AbstractSampler;

/*!
 * \ingroup SW
 */
class ProgramSw : public IProgram
{
	T_RTTI_CLASS;

public:
	ProgramSw(
		const std::map< handle_t, std::pair< int, int > >& parameterMap,
		const std::map< handle_t, int >& samplerMap,
		Processor::image_t vertexProgram,
		Processor::image_t pixelProgram,
		const RenderStateDesc& renderState,
		uint32_t interpolatorCount
	);

	virtual void destroy();

	virtual void setFloatParameter(handle_t handle, float param);

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length);
	
	virtual void setVectorParameter(handle_t handle, const Vector4& param);

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length);

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param);

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length);

	virtual void setTextureParameter(handle_t handle, ITexture* texture);

	virtual void setStencilReference(uint32_t stencilReference);

	inline Processor::image_t getVertexProgram() const {
		return m_vertexProgram;
	}

	inline Processor::image_t getPixelProgram() const {
		return m_pixelProgram;
	}

	inline const RenderStateDesc& getRenderState() const {
		return m_renderState;
	}

	inline uint32_t getInterpolatorCount() const {
		return m_interpolatorCount;
	}

	inline const Vector4* getParameters() const {
		return m_parameters;
	}

	inline const Ref< AbstractSampler >* getSamplers() const {
		return m_samplers;
	}

private:
	Processor::image_t m_vertexProgram;
	Processor::image_t m_pixelProgram;
	RenderStateDesc m_renderState;
	uint32_t m_interpolatorCount;
	std::map< handle_t, std::pair< int, int > > m_parameterMap;
	std::map< handle_t, int > m_samplerMap;
	Vector4* m_parameters;
	Ref< AbstractSampler > m_samplers[16];
};

	}
}

#endif	// traktor_render_ProgramSw_H
