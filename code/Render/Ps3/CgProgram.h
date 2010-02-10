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
		const std::vector< std::wstring >& vertexTextures,
		const std::vector< std::wstring >& pixelTextures,
		const RenderState& renderState
	);

	const std::wstring& getVertexShader() const;

	const std::wstring& getPixelShader() const;

	const std::vector< std::wstring >& getVertexTextures() const;

	const std::vector< std::wstring >& getPixelTextures() const;

	const RenderState& getRenderState() const;

private:
	std::wstring m_vertexShader;
	std::wstring m_pixelShader;
	std::vector< std::wstring > m_vertexTextures;
	std::vector< std::wstring > m_pixelTextures;
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_CgProgram_H
