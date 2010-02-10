#ifndef traktor_render_ProgramResourceDx10_H
#define traktor_render_ProgramResourceDx10_H

#include "Render/Ps3/TypesPs3.h"
#include "Render/Resource/ProgramResource.h"

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

	ProgramResourcePs3();

	ProgramResourcePs3(
		CGCbin* vertexShaderBin,
		CGCbin* pixelShaderBin,
		const std::vector< Parameter >& parameters,
		const std::vector< std::wstring >& vertexTextures,
		const std::vector< std::wstring >& pixelTextures,
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

	const std::vector< std::wstring >& getVertexTextures() const {
		return m_vertexTextures;
	}

	const std::vector< std::wstring >& getPixelTextures() const {
		return m_pixelTextures;
	}

	const RenderState& getRenderState() const {
		return m_renderState;
	}

private:
	CGCbin* m_vertexShaderBin;
	CGCbin* m_pixelShaderBin;
	std::vector< Parameter > m_parameters;
	std::vector< std::wstring > m_vertexTextures;
	std::vector< std::wstring > m_pixelTextures;
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_ProgramResourceDx10_H
