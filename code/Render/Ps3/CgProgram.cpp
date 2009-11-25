#include "Render/Ps3/CgProgram.h"

namespace traktor
{
	namespace render
	{

CgProgram::CgProgram()
{
}

CgProgram::CgProgram(
	const std::wstring& vertexShader,
	const std::wstring& pixelShader,
	const RenderState& renderState
)
:	m_vertexShader(vertexShader)
,	m_pixelShader(pixelShader)
,	m_renderState(renderState)
{
}

const std::wstring& CgProgram::getVertexShader() const
{
	return m_vertexShader;
}

const std::wstring& CgProgram::getPixelShader() const
{
	return m_pixelShader;
}

const RenderState& CgProgram::getRenderState() const
{
	return m_renderState;
}

	}
}
