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
	const std::vector< std::wstring >& textures,
	const std::vector< SamplerBinding >& samplers,
	const RenderState& renderState
)
:	m_vertexShader(vertexShader)
,	m_fragmentShader(fragmentShader)
,	m_textures(textures)
,	m_samplers(samplers)
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

const std::vector< std::wstring >& GlslProgram::getTextures() const
{
	return m_textures;
}

const std::vector< SamplerBinding>& GlslProgram::getSamplers() const
{
	return m_samplers;
}

const RenderState& GlslProgram::getRenderState() const
{
	return m_renderState;
}

	}
}
