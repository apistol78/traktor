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
		const std::set< std::wstring >& vertexSamplers,
		const std::set< std::wstring >& fragmentSamplers,
		const RenderState& renderState
	);

	const std::wstring& getVertexShader() const;

	const std::wstring& getFragmentShader() const;

	const std::set< std::wstring >& getVertexSamplers() const;

	const std::set< std::wstring >& getFragmentSamplers() const;

	const RenderState& getRenderState() const;

private:
	std::wstring m_vertexShader;
	std::wstring m_fragmentShader;
	std::set< std::wstring > m_vertexSamplers;
	std::set< std::wstring > m_fragmentSamplers;
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_GlslProgram_H
