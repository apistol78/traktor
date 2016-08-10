#include "Render/Dx9/Hlsl/HlslProgram.h"

namespace traktor
{
	namespace render
	{

HlslProgram::HlslProgram()
{
}

HlslProgram::HlslProgram(
	const std::wstring& vertexShader,
	const std::wstring& pixelShader,
	const std::set< std::wstring >& vertexTextures,
	const std::map< uint32_t, std::pair< std::wstring, int32_t > >& vertexSamplers,
	const std::set< std::wstring >& pixelTextures,
	const std::map< uint32_t, std::pair< std::wstring, int32_t > >& pixelSamplers,
	const StateBlockDx9& state
)
:	m_vertexShader(vertexShader)
,	m_pixelShader(pixelShader)
,	m_vertexTextures(vertexTextures)
,	m_vertexSamplers(vertexSamplers)
,	m_pixelTextures(pixelTextures)
,	m_pixelSamplers(pixelSamplers)
,	m_state(state)
{
}

const std::wstring& HlslProgram::getVertexShader() const
{
	return m_vertexShader;
}

const std::wstring& HlslProgram::getPixelShader() const
{
	return m_pixelShader;
}

const std::set< std::wstring >& HlslProgram::getVertexTextures() const
{
	return m_vertexTextures;
}

const std::map< uint32_t, std::pair< std::wstring, int32_t > >& HlslProgram::getVertexSamplers() const
{
	return m_vertexSamplers;
}

const std::set< std::wstring >& HlslProgram::getPixelTextures() const
{
	return m_pixelTextures;
}

const std::map< uint32_t, std::pair< std::wstring, int32_t > >& HlslProgram::getPixelSamplers() const
{
	return m_pixelSamplers;
}

const StateBlockDx9& HlslProgram::getState() const
{
	return m_state;
}

	}
}
