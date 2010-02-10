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
	const std::vector< std::wstring >& vertexTextures,
	const std::vector< std::wstring >& pixelTextures,
	const RenderState& renderState
)
:	m_vertexShader(vertexShader)
,	m_pixelShader(pixelShader)
,	m_vertexTextures(vertexTextures)
,	m_pixelTextures(pixelTextures)
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

const std::vector< std::wstring >& CgProgram::getVertexTextures() const
{
	return m_vertexTextures;
}

const std::vector< std::wstring >& CgProgram::getPixelTextures() const
{
	return m_pixelTextures;
}

const RenderState& CgProgram::getRenderState() const
{
	return m_renderState;
}

	}
}
