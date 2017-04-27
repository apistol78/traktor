/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IResourceDx9_H
#define traktor_render_IResourceDx9_H

#include "Render/Dx9/Platform.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX9 Xbox360
 */
class IResourceDx9
{
public:
	virtual HRESULT lostDevice() = 0;

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice) = 0;
};

	}
}

#endif	// traktor_render_IResourceDx9_H
