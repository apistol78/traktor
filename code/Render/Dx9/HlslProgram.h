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
		const std::map< std::wstring, int32_t >& vertexTextures,
		const std::map< std::wstring, int32_t >& pixelTextures,
		const StateBlockDx9& state
	);

	const std::wstring& getVertexShader() const;

	const std::wstring& getPixelShader() const;

	const std::map< std::wstring, int32_t >& getVertexTextures() const;

	const std::map< std::wstring, int32_t >& getPixelTextures() const;

	const StateBlockDx9& getState() const;

private:
	std::wstring m_vertexShader;
	std::wstring m_pixelShader;
	std::map< std::wstring, int32_t > m_vertexTextures;
	std::map< std::wstring, int32_t > m_pixelTextures;
	StateBlockDx9 m_state;
};

	}
}

#endif	// traktor_render_HlslProgram_H
