/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ClearTarget_H
#define traktor_render_ClearTarget_H

#include "Core/Object.h"
#include "Core/Math/Color4f.h"
#include "Core/Misc/ComRef.h"
#include "Render/Dx9/Platform.h"

namespace traktor
{
	namespace render
	{

class ParameterCache;

class ClearTarget : public Object
{
	T_RTTI_CLASS;

public:
	bool create(IDirect3DDevice9* d3dDevice);

	bool clear(IDirect3DDevice9* d3dDevice, ParameterCache* parameterCache, int32_t width, int32_t height, const Color4f* colors, int32_t ntargets);

private:
	ComRef< ID3DXEffect > m_d3dClearEffect;
	D3DXHANDLE m_d3dClearTechnique[2];
	D3DXHANDLE m_d3dClearColor[2];
};

	}
}

#endif	// traktor_render_ClearTarget_H
