#ifndef traktor_render_ProgramResourceDx11_H
#define traktor_render_ProgramResourceDx11_H

#include <map>
#include "Core/Misc/ComRef.h"
#include "Render/Dx11/Platform.h"
#include "Render/Resource/ProgramResource.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX11
 */
class ProgramResourceDx11 : public ProgramResource
{
	T_RTTI_CLASS;

public:
	ProgramResourceDx11();

	virtual void serialize(ISerializer& s);

private:
	friend class ProgramDx11;
	friend class ProgramCompilerDx11;

	ComRef< ID3DBlob > m_vertexShader;
	ComRef< ID3DBlob > m_pixelShader;
	uint32_t m_vertexShaderHash;
	uint32_t m_pixelShaderHash;
	D3D11_RASTERIZER_DESC m_d3dRasterizerDesc;
	D3D11_DEPTH_STENCIL_DESC m_d3dDepthStencilDesc;
	D3D11_BLEND_DESC m_d3dBlendDesc;
	uint32_t m_stencilReference;
	std::map< std::wstring, D3D11_SAMPLER_DESC > m_d3dVertexSamplers;
	std::map< std::wstring, D3D11_SAMPLER_DESC > m_d3dPixelSamplers;
};

	}
}

#endif	// traktor_render_ProgramResourceDx11_H
