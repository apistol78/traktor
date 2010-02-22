#ifndef traktor_render_ParameterCache_H
#define traktor_render_ParameterCache_H

#include "Core/Object.h"
#include "Core/Misc/ComRef.h"
#include "Render/Dx9/Platform.h"

namespace traktor
{
	namespace render
	{

class ParameterCache : public Object
{
public:
	enum
	{
		VertexConstantCount = 256,
		PixelConstantCount = 224,
		VertexTextureCount = 8,
		PixelTextureCount = 8,
		MaxTextureCount = 8
	};

	ParameterCache(IDirect3DDevice9* d3dDevice);

	virtual ~ParameterCache();

	void setVertexShader(IDirect3DVertexShader9* d3dVertexShader);

	void setPixelShader(IDirect3DPixelShader9* d3dPixelShader);

	void setVertexShaderConstant(uint32_t registerOffset, uint32_t registerCount, const float* constantData);

	void setPixelShaderConstant(uint32_t registerOffset, uint32_t registerCount, const float* constantData);

	void setVertexTexture(uint32_t stage, IDirect3DBaseTexture9* d3dTexture);

	void setPixelTexture(uint32_t stage, IDirect3DBaseTexture9* d3dTexture);

	void setRenderState(uint32_t state, uint32_t value);

	void setSamplerState(uint32_t sampler, uint32_t state, uint32_t value);

	HRESULT lostDevice();

	HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

private:
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	IDirect3DVertexShader9* m_d3dVertexShader;
	IDirect3DPixelShader9* m_d3dPixelShader;
	float* m_vertexConstantsShadow;
	float* m_pixelConstantsShadow;
	IDirect3DBaseTexture9* m_vertexTextureShadow[VertexTextureCount];
	IDirect3DBaseTexture9* m_pixelTextureShadow[PixelTextureCount];
	std::vector< uint32_t > m_renderStates;
	std::vector< uint32_t > m_samplerStates[MaxTextureCount];
};

	}
}

#endif	// traktor_render_ParameterCache_H
