#ifndef traktor_render_RenderTargetSetSw_H
#define traktor_render_RenderTargetSetSw_H

#include <vector>
#include "Core/RefArray.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/RenderTargetSet.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_SW_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

struct RenderTargetSetCreateDesc;
class RenderTargetSw;

/*!
 * \ingroup SW
 */
class T_DLLCLASS RenderTargetSetSw : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	RenderTargetSetSw();

	bool create(const RenderTargetSetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual Ref< ITexture > getColorTexture(int index) const;

	virtual void swap(int index1, int index2);

	virtual bool read(int index, void* buffer) const;

	uint16_t* getDepthSurface();

private:
	RefArray< RenderTargetSw > m_colorTargets;
	AutoArrayPtr< uint16_t > m_depthSurface;
	int m_width;
	int m_height;
};

	}
}

#endif	// traktor_render_RenderTargetSetSw_H
