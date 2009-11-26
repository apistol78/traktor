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
	const std::map< std::wstring, uint32_t >& vertexSamplers,
	const std::map< std::wstring, uint32_t >& pixelSamplers,
	const RenderState& renderState
)
:	m_vertexShader(vertexShader)
,	m_pixelShader(pixelShader)
,	m_vertexSamplers(vertexSamplers)
,	m_pixelSamplers(pixelSamplers)
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

const std::map< std::wstring, uint32_t >& CgProgram::getVertexSamplers() const
{
	return m_vertexSamplers;
}

const std::map< std::wstring, uint32_t >& CgProgram::getPixelSamplers() const
{
	return m_pixelSamplers;
}

const RenderState& CgProgram::getRenderState() const
{
	return m_renderState;
}

	}
}
