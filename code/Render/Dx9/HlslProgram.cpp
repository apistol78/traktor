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
	const StateBlockDx9& state
)
:	m_vertexShader(vertexShader)
,	m_pixelShader(pixelShader)
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

const StateBlockDx9& HlslProgram::getState() const
{
	return m_state;
}

	}
}
