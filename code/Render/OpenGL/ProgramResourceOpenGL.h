#ifndef traktor_render_ProgramResourceOpenGL_H
#define traktor_render_ProgramResourceOpenGL_H

#include "Render/OpenGL/TypesOpenGL.h"
#include "Render/Resource/ProgramResource.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup OGL
 */
class ProgramResourceOpenGL : public ProgramResource
{
	T_RTTI_CLASS;

public:
	ProgramResourceOpenGL();

	ProgramResourceOpenGL(
		const std::wstring& vertexShader,
		const std::wstring& fragmentShader,
		const std::vector< SamplerTexture >& samplerTextures,
		const RenderState& renderState
	);

	const std::wstring& getVertexShader() const { return m_vertexShader; }

	const std::wstring& getFragmentShader() const { return m_fragmentShader; }

	const std::vector< SamplerTexture >& getSamplerTextures() const { return m_samplerTextures; }

	const RenderState& getRenderState() const { return m_renderState; }

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_vertexShader;
	std::wstring m_fragmentShader;
	std::vector< SamplerTexture > m_samplerTextures;
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_ProgramResourceOpenGL_H
