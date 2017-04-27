/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Dx11/Hlsl/HlslProgram.h"

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
	const D3D11_RASTERIZER_DESC& d3dRasterizerDesc,
	const D3D11_DEPTH_STENCIL_DESC& d3dDepthStencilDesc,
	const D3D11_BLEND_DESC& d3dBlendDesc,
	uint32_t stencilReference,
	const std::map< std::wstring, D3D11_SAMPLER_DESC >& d3dVertexSamplers,
	const std::map< std::wstring, D3D11_SAMPLER_DESC >& d3dPixelSamplers
)
:	m_vertexShader(vertexShader)
,	m_pixelShader(pixelShader)
,	m_d3dRasterizerDesc(d3dRasterizerDesc)
,	m_d3dDepthStencilDesc(d3dDepthStencilDesc)
,	m_d3dBlendDesc(d3dBlendDesc)
,	m_stencilReference(stencilReference)
,	m_d3dVertexSamplers(d3dVertexSamplers)
,	m_d3dPixelSamplers(d3dPixelSamplers)
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

const D3D11_RASTERIZER_DESC& HlslProgram::getD3DRasterizerDesc() const
{
	return m_d3dRasterizerDesc;
}

const D3D11_DEPTH_STENCIL_DESC& HlslProgram::getD3DDepthStencilDesc() const
{
	return m_d3dDepthStencilDesc;
}

const D3D11_BLEND_DESC& HlslProgram::getD3DBlendDesc() const
{
	return m_d3dBlendDesc;
}

uint32_t HlslProgram::getStencilReference() const
{
	return m_stencilReference;
}

const std::map< std::wstring, D3D11_SAMPLER_DESC >& HlslProgram::getD3DVertexSamplers() const
{
	return m_d3dVertexSamplers;
}

const std::map< std::wstring, D3D11_SAMPLER_DESC >& HlslProgram::getD3DPixelSamplers() const
{
	return m_d3dPixelSamplers;
}

	}
}
