#ifndef traktor_render_CgProgram_H
#define traktor_render_CgProgram_H

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
		const RenderState& renderState
	);

	const std::wstring& getVertexShader() const;

	const std::wstring& getPixelShader() const;

	const RenderState& getRenderState() const;

private:
	std::wstring m_vertexShader;
	std::wstring m_pixelShader;
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_CgProgram_H
