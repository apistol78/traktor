/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_HlslProgram_H
#define traktor_render_HlslProgram_H

#include <string>
#include <map>
#include "Core/Config.h"
#include "Render/Dx11/Platform.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX11
 */
class HlslProgram
{
public:
	HlslProgram();

	HlslProgram(
		const std::wstring& vertexShader,
		const std::wstring& pixelShader,
		const D3D11_RASTERIZER_DESC& d3dRasterizerDesc,
		const D3D11_DEPTH_STENCIL_DESC& d3dDepthStencilDesc,
		const D3D11_BLEND_DESC& d3dBlendDesc,
		uint32_t stencilReference,
		const std::map< std::wstring, D3D11_SAMPLER_DESC >& d3dVertexSamplers,
		const std::map< std::wstring, D3D11_SAMPLER_DESC >& d3dPixelSamplers
	);

	const std::wstring& getVertexShader() const;

	const std::wstring& getPixelShader() const;

	const D3D11_RASTERIZER_DESC& getD3DRasterizerDesc() const;

	const D3D11_DEPTH_STENCIL_DESC& getD3DDepthStencilDesc() const;

	const D3D11_BLEND_DESC& getD3DBlendDesc() const;

	uint32_t getStencilReference() const;

	const std::map< std::wstring, D3D11_SAMPLER_DESC >& getD3DVertexSamplers() const;

	const std::map< std::wstring, D3D11_SAMPLER_DESC >& getD3DPixelSamplers() const;

private:
	std::wstring m_vertexShader;
	std::wstring m_pixelShader;
	D3D11_RASTERIZER_DESC m_d3dRasterizerDesc;
	D3D11_DEPTH_STENCIL_DESC m_d3dDepthStencilDesc;
	D3D11_BLEND_DESC m_d3dBlendDesc;
	uint32_t m_stencilReference;
	std::map< std::wstring, D3D11_SAMPLER_DESC > m_d3dVertexSamplers;
	std::map< std::wstring, D3D11_SAMPLER_DESC > m_d3dPixelSamplers;
};

	}
}

#endif	// traktor_render_HlslProgram_H
