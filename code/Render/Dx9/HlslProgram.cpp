#include "Render/Dx9/HlslProgram.h"

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
	const std::map< std::wstring, int32_t >& vertexTextures,
	const std::map< std::wstring, int32_t >& pixelTextures,
	const StateBlockDx9& state
)
:	m_vertexShader(vertexShader)
,	m_pixelShader(pixelShader)
,	m_vertexTextures(vertexTextures)
,	m_pixelTextures(pixelTextures)
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

const std::map< std::wstring, int32_t >& HlslProgram::getVertexTextures() const
{
	return m_vertexTextures;
}

const std::map< std::wstring, int32_t >& HlslProgram::getPixelTextures() const
{
	return m_pixelTextures;
}

const StateBlockDx9& HlslProgram::getState() const
{
	return m_state;
}

	}
}
