#ifndef traktor_render_ProgramResourceOpenGL_H
#define traktor_render_ProgramResourceOpenGL_H

#include "Render/ProgramResource.h"
#include "Render/OpenGL/TypesOpenGL.h"

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
		const std::set< std::wstring >& vertexSamplers,
		const std::set< std::wstring >& fragmentSamplers,
		const RenderState& renderState
	);

	const std::wstring& getVertexShader() const { return m_vertexShader; }

	const std::wstring& getFragmentShader() const { return m_fragmentShader; }

	const std::set< std::wstring >& getVertexSamplers() const { return m_vertexSamplers; }

	const std::set< std::wstring >& getFragmentSamplers() const { return m_fragmentSamplers; }

	const RenderState& getRenderState() const { return m_renderState; }

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_vertexShader;
	std::wstring m_fragmentShader;
	std::set< std::wstring > m_vertexSamplers;
	std::set< std::wstring > m_fragmentSamplers;
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_ProgramResourceOpenGL_H
