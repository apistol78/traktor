/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderTargetPool_H
#define traktor_render_RenderTargetPool_H

#include <vector>
#include "Core/Object.h"
#include "Core/Misc/ComRef.h"
#include "Render/Dx9/Platform.h"

namespace traktor
{
	namespace render
	{

/*! \brief Render target pool.
 * \ingroup Xbox360
 *
 * As the X-box has limited EDRAM this pool will manage
 * shared physical render targets by reusing "free" targets.
 * In order to minimize chance of having to allocate a new target
 * this pool use a metric based on target's size to choose the
 * optimal target to fit the requested size.
 */
class RenderTargetPool : public Object
{
	T_RTTI_CLASS;

public:
	struct Target
	{
		D3DFORMAT d3dFormat;
		D3DMULTISAMPLE_TYPE d3dMultisample;
		int width;
		int height;
		IDirect3DSurface9* d3dSurface;
	};

	RenderTargetPool();

	void destroy(IDirect3DDevice9* d3dDevice);

	Target* acquire(IDirect3DDevice9* d3dDevice, int width, int height, D3DFORMAT d3dFormat, D3DMULTISAMPLE_TYPE d3dMultisample);

	void release(Target*& target);

private:
	std::vector< Target* > m_targetPool;
	uint32_t m_edramBase;
};

	}
}

#endif	// traktor_render_RenderTargetPool_H
