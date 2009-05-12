#include "Render/OpenGL/GlslProgram.h"

namespace traktor
{
	namespace render
	{

GlslProgram::GlslProgram()
{
}

GlslProgram::GlslProgram(
	const std::wstring& vertexShader,
	const std::wstring& fragmentShader,
	const std::set< std::wstring >& vertexSamplers,
	const std::set< std::wstring >& fragmentSamplers,
	const RenderState& renderState
)
:	m_vertexShader(vertexShader)
,	m_fragmentShader(fragmentShader)
,	m_vertexSamplers(vertexSamplers)
,	m_fragmentSamplers(fragmentSamplers)
,	m_renderState(renderState)
{
}

const std::wstring& GlslProgram::getVertexShader() const
{
	return m_vertexShader;
}

const std::wstring& GlslProgram::getFragmentShader() const
{
	return m_fragmentShader;
}

const std::set< std::wstring >& GlslProgram::getVertexSamplers() const
{
	return m_vertexSamplers;
}

const std::set< std::wstring >& GlslProgram::getFragmentSamplers() const
{
	return m_fragmentSamplers;
}

const RenderState& GlslProgram::getRenderState() const
{
	return m_renderState;
}

	}
}
