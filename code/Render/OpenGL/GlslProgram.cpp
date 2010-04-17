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
	const std::map< std::wstring, int32_t >& samplerTextures,
	const RenderState& renderState
)
:	m_vertexShader(vertexShader)
,	m_fragmentShader(fragmentShader)
,	m_samplerTextures(samplerTextures)
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

const std::map< std::wstring, int32_t >& GlslProgram::getSamplerTextures() const
{
	return m_samplerTextures;
}

const RenderState& GlslProgram::getRenderState() const
{
	return m_renderState;
}

	}
}
