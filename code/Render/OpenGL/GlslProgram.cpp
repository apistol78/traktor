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
	const std::vector< NamedUniformType >& uniforms,
	const std::vector< SamplerBindingOpenGL >& samplers,
	const RenderStateOpenGL& renderState
)
:	m_vertexShader(vertexShader)
,	m_fragmentShader(fragmentShader)
,	m_textures(textures)
,	m_uniforms(uniforms)
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

const std::vector< NamedUniformType >& GlslProgram::getUniforms() const
{
	return m_uniforms;
}

const std::vector< SamplerBindingOpenGL >& GlslProgram::getSamplers() const
{
	return m_samplers;
}

const RenderStateOpenGL& GlslProgram::getRenderState() const
{
	return m_renderState;
}

	}
}
