#ifndef traktor_render_GlslProgram_H
#define traktor_render_GlslProgram_H

#include <set>
#include <string>
#include "Render/OpenGL/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup OGL
 */
class GlslProgram
{
public:
	GlslProgram();

	GlslProgram(
		const std::wstring& vertexShader,
		const std::wstring& fragmentShader,
		const std::vector< SamplerTexture >& samplerTextures,
		const RenderState& renderState
	);

	const std::wstring& getVertexShader() const;

	const std::wstring& getFragmentShader() const;

	const std::vector< SamplerTexture >& getSamplerTextures() const;

	const RenderState& getRenderState() const;

private:
	std::wstring m_vertexShader;
	std::wstring m_fragmentShader;
	std::vector< SamplerTexture > m_samplerTextures;
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_GlslProgram_H
