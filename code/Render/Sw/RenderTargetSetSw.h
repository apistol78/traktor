#ifndef traktor_render_RenderTargetSetSw_H
#define traktor_render_RenderTargetSetSw_H

#include <vector>
#include "Core/RefArray.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/RenderTargetSet.h"

namespace traktor
{
	namespace render
	{

struct RenderTargetSetCreateDesc;
class RenderTargetSw;

/*!
 * \ingroup SW
 */
class RenderTargetSetSw : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	RenderTargetSetSw();

	bool create(const RenderTargetSetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual ISimpleTexture* getColorTexture(int index) const;

	virtual void swap(int index1, int index2);

	virtual bool read(int index, void* buffer) const;

	float* getDepthSurface() { return m_depthSurface.ptr(); }

	uint8_t* getStencilSurface() { return m_stencilSurface.ptr(); }

	bool usingPrimaryDepth() const { return m_usingPrimaryDepth; }

private:
	RefArray< RenderTargetSw > m_colorTargets;
	AutoArrayPtr< float, AllocFreeAlign > m_depthSurface;
	AutoArrayPtr< uint8_t, AllocFreeAlign > m_stencilSurface;
	int m_width;
	int m_height;
	bool m_usingPrimaryDepth;
};

	}
}

#endif	// traktor_render_RenderTargetSetSw_H
