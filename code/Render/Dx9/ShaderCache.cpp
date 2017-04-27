/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Dx9/ShaderCache.h"

namespace traktor
{
	namespace render
	{

void ShaderCache::releaseAll()
{
	m_d3dVertexShaders.clear();
	m_d3dPixelShaders.clear();
}

void ShaderCache::putVertexShader(uint32_t vertexShaderHash, IDirect3DVertexShader9* d3dVertexShader)
{
	m_d3dVertexShaders[vertexShaderHash] = d3dVertexShader;
}

void ShaderCache::putPixelShader(uint32_t pixelShaderHash, IDirect3DPixelShader9* d3dPixelShader)
{
	m_d3dPixelShaders[pixelShaderHash] = d3dPixelShader;
}

IDirect3DVertexShader9* ShaderCache::getVertexShader(uint32_t vertexShaderHash) const
{
	std::map< uint32_t, ComRef< IDirect3DVertexShader9 > >::const_iterator i = m_d3dVertexShaders.find(vertexShaderHash);
	return i != m_d3dVertexShaders.end() ? i->second : 0;
}

IDirect3DPixelShader9* ShaderCache::getPixelShader(uint32_t pixelShaderHash) const
{
	std::map< uint32_t, ComRef< IDirect3DPixelShader9 > >::const_iterator i = m_d3dPixelShaders.find(pixelShaderHash);
	return i != m_d3dPixelShaders.end() ? i->second : 0;
}

	}
}
