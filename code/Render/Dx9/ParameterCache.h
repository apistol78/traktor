#ifndef traktor_render_ParameterCache_H
#define traktor_render_ParameterCache_H

#include "Core/Config.h"
#include "Core/Misc/ComRef.h"
#include "Render/Dx9/Unmanaged.h"

namespace traktor
{
	namespace render
	{

class ParameterCache : public Unmanaged
{
public:
	ParameterCache(UnmanagedListener* listener, IDirect3DDevice9* d3dDevice);

	virtual ~ParameterCache();

	void setVertexShader(IDirect3DVertexShader9* d3dVertexShader);

	void setPixelShader(IDirect3DPixelShader9* d3dPixelShader);

	void setVertexShaderConstant(uint32_t registerOffset, uint32_t registerCount, const float* constantData);

	void setPixelShaderConstant(uint32_t registerOffset, uint32_t registerCount, const float* constantData);

	void setVertexTexture(uint32_t stage, IDirect3DBaseTexture9* d3dTexture);

	void setPixelTexture(uint32_t stage, IDirect3DBaseTexture9* d3dTexture);

	void setRenderState(uint32_t state, uint32_t value);

	void setSamplerState(uint32_t sampler, uint32_t state, uint32_t value);

protected:
	virtual HRESULT lostDevice();

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

private:
	enum
	{
		VertexConstantCount = 256,
		PixelConstantCount = 224,
		VertexTextureCount = 8,
		PixelTextureCount = 8,
		MaxTextureCount = 8
	};

	ComRef< IDirect3DDevice9 > m_d3dDevice;
	ComRef< IDirect3DVertexShader9 > m_d3dVertexShader;
	ComRef< IDirect3DPixelShader9 > m_d3dPixelShader;
	float* m_vertexConstantsShadow;
	float* m_pixelConstantsShadow;
	ComRef< IDirect3DBaseTexture9 > m_vertexTextureShadow[VertexTextureCount];
	ComRef< IDirect3DBaseTexture9 > m_pixelTextureShadow[PixelTextureCount];
	std::vector< uint32_t > m_renderStates;
	std::vector< uint32_t > m_samplerStates[MaxTextureCount];
};

	}
}

#endif	// traktor_render_ParameterCache_H
