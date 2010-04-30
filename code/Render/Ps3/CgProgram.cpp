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
	const std::map< std::wstring, int32_t >& vertexTextures,
	const std::map< std::wstring, int32_t >& pixelTextures,
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

const std::map< std::wstring, int32_t >& CgProgram::getVertexTextures() const
{
	return m_vertexTextures;
}

const std::map< std::wstring, int32_t >& CgProgram::getPixelTextures() const
{
	return m_pixelTextures;
}

const RenderState& CgProgram::getRenderState() const
{
	return m_renderState;
}

	}
}
