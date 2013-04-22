#ifndef traktor_render_ProgramResourceDx10_H
#define traktor_render_ProgramResourceDx10_H

#include <map>
#include "Core/Misc/ComRef.h"
#include "Render/Dx10/Platform.h"
#include "Render/Resource/ProgramResource.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX10
 */
class ProgramResourceDx10 : public ProgramResource
{
	T_RTTI_CLASS;

public:
	ProgramResourceDx10();

	virtual bool serialize(ISerializer& s);

private:
	friend class ProgramDx10;

	ComRef< ID3D10Blob > m_vertexShader;
	ComRef< ID3D10Blob > m_pixelShader;
	D3D10_RASTERIZER_DESC m_d3dRasterizerDesc;
	D3D10_DEPTH_STENCIL_DESC m_d3dDepthStencilDesc;
	D3D10_BLEND_DESC m_d3dBlendDesc;
	uint32_t m_stencilReference;
	std::map< std::wstring, D3D10_SAMPLER_DESC > m_d3dVertexSamplers;
	std::map< std::wstring, D3D10_SAMPLER_DESC > m_d3dPixelSamplers;
};

	}
}

#endif	// traktor_render_ProgramResourceDx10_H
