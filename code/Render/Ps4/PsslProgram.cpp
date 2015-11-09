#include "Render/Ps4/PsslProgram.h"

namespace traktor
{
	namespace render
	{

PsslProgram::PsslProgram()
{
}

PsslProgram::PsslProgram(
	const std::wstring& vertexShader,
	const std::wstring& pixelShader
)
:	m_vertexShader(vertexShader)
,	m_pixelShader(pixelShader)
{
}

const std::wstring& PsslProgram::getVertexShader() const
{
	return m_vertexShader;
}

const std::wstring& PsslProgram::getPixelShader() const
{
	return m_pixelShader;
}

	}
}
