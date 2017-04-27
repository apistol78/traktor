/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ShaderCache_H
#define traktor_render_ShaderCache_H

#include <map>
#include "Core/Object.h"
#include "Core/Misc/ComRef.h"
#include "Render/Dx9/TypesDx9.h"

namespace traktor
{
	namespace render
	{

class ShaderCache : public Object
{
public:
	void releaseAll();

	void putVertexShader(uint32_t vertexShaderHash, IDirect3DVertexShader9* d3dVertexShader);

	void putPixelShader(uint32_t pixelShaderHash, IDirect3DPixelShader9* d3dPixelShader);

	IDirect3DVertexShader9* getVertexShader(uint32_t vertexShaderHash) const;

	IDirect3DPixelShader9* getPixelShader(uint32_t pixelShaderHash) const;

private:
	std::map< uint32_t, ComRef< IDirect3DVertexShader9 > > m_d3dVertexShaders;
	std::map< uint32_t, ComRef< IDirect3DPixelShader9 > > m_d3dPixelShaders;
};

	}
}

#endif	// traktor_render_ShaderCache_H
