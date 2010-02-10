#ifndef traktor_render_HlslProgram_H
#define traktor_render_HlslProgram_H

#include "Render/Dx9/StateBlockDx9.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX9 Xbox360
 */
class HlslProgram
{
public:
	HlslProgram();

	HlslProgram(
		const std::wstring& vertexShader,
		const std::wstring& pixelShader,
		const std::vector< std::wstring >& vertexTextures,
		const std::vector< std::wstring >& pixelTextures,
		const StateBlockDx9& state
	);

	const std::wstring& getVertexShader() const;

	const std::wstring& getPixelShader() const;

	const std::vector< std::wstring >& getVertexTextures() const;

	const std::vector< std::wstring >& getPixelTextures() const;

	const StateBlockDx9& getState() const;

private:
	std::wstring m_vertexShader;
	std::wstring m_pixelShader;
	std::vector< std::wstring > m_vertexTextures;
	std::vector< std::wstring > m_pixelTextures;
	StateBlockDx9 m_state;
};

	}
}

#endif	// traktor_render_HlslProgram_H
