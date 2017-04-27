/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Ps3/Cg/CgProgram.h"

namespace traktor
{
	namespace render
	{

CgProgram::CgProgram()
:	m_registerCount(0)
{
}

CgProgram::CgProgram(
	const std::wstring& vertexShader,
	const std::wstring& pixelShader,
	const std::map< std::wstring, int32_t >& vertexTextures,
	const std::map< std::wstring, int32_t >& pixelTextures,
	const RenderStateGCM& renderState,
	uint32_t registerCount
)
:	m_vertexShader(vertexShader)
,	m_pixelShader(pixelShader)
,	m_vertexTextures(vertexTextures)
,	m_pixelTextures(pixelTextures)
,	m_renderState(renderState)
,	m_registerCount(registerCount)
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

const RenderStateGCM& CgProgram::getRenderState() const
{
	return m_renderState;
}

uint32_t CgProgram::getRegisterCount() const
{
	return m_registerCount;
}

	}
}
