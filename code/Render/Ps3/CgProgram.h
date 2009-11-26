#ifndef traktor_render_CgProgram_H
#define traktor_render_CgProgram_H

#include <map>
#include "Render/Ps3/TypesPs3.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup PS3
 */
class CgProgram
{
public:
	CgProgram();

	CgProgram(
		const std::wstring& vertexShader,
		const std::wstring& pixelShader,
		const std::map< std::wstring, uint32_t >& vertexSamplers,
		const std::map< std::wstring, uint32_t >& pixelSamplers,
		const RenderState& renderState
	);

	const std::wstring& getVertexShader() const;

	const std::wstring& getPixelShader() const;

	const std::map< std::wstring, uint32_t >& getVertexSamplers() const;

	const std::map< std::wstring, uint32_t >& getPixelSamplers() const;

	const RenderState& getRenderState() const;

private:
	std::wstring m_vertexShader;
	std::wstring m_pixelShader;
	std::map< std::wstring, uint32_t > m_vertexSamplers;
	std::map< std::wstring, uint32_t > m_pixelSamplers;
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_CgProgram_H
