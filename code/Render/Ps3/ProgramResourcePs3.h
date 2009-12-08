#ifndef traktor_render_ProgramResourceDx10_H
#define traktor_render_ProgramResourceDx10_H

#include "Render/Ps3/TypesPs3.h"
#include "Render/ProgramResource.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup PS3
 */
class ProgramResourcePs3 : public ProgramResource
{
	T_RTTI_CLASS;

public:
	struct Parameter
	{
		std::wstring name;
		int32_t size;		//< Number of elements, ie float4 == 4, float4x4 == 16 etc.
		int32_t count;		//< Size of indexed uniforms.

		bool serialize(ISerializer& s);
	};

	struct Sampler
	{
		std::wstring name;
		uint32_t stage;

		bool serialize(ISerializer& s);
	};

	ProgramResourcePs3();

	ProgramResourcePs3(
		CGCbin* vertexShaderBin,
		CGCbin* pixelShaderBin,
		const std::vector< Parameter >& parameters,
		const std::vector< Sampler >& vertexSamplers,
		const std::vector< Sampler >& pixelSamplers,
		const RenderState& renderState
	);

	virtual ~ProgramResourcePs3();

	virtual bool serialize(ISerializer& s);

	CGCbin* getVertexShaderBin() const {
		return m_vertexShaderBin;
	}

	CGCbin* getPixelShaderBin() const {
		return m_pixelShaderBin;
	}

	const std::vector< Parameter >& getParameters() const {
		return m_parameters;
	}

	const std::vector< Sampler >& getVertexSamplers() const {
		return m_vertexSamplers;
	}

	const std::vector< Sampler >& getPixelSamplers() const {
		return m_pixelSamplers;
	}

	const RenderState& getRenderState() const {
		return m_renderState;
	}

private:
	CGCbin* m_vertexShaderBin;
	CGCbin* m_pixelShaderBin;
	std::vector< Parameter > m_parameters;
	std::vector< Sampler > m_vertexSamplers;
	std::vector< Sampler > m_pixelSamplers;
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_ProgramResourceDx10_H
