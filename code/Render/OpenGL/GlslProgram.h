#ifndef traktor_render_GlslProgram_H
#define traktor_render_GlslProgram_H

#include <map>
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
		const std::map< std::wstring, int32_t >& samplerTextures,
		const RenderState& renderState
	);

	const std::wstring& getVertexShader() const;

	const std::wstring& getFragmentShader() const;

	const std::map< std::wstring, int32_t >& getSamplerTextures() const;

	const RenderState& getRenderState() const;

private:
	std::wstring m_vertexShader;
	std::wstring m_fragmentShader;
	std::map< std::wstring, int32_t > m_samplerTextures;
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_GlslProgram_H
