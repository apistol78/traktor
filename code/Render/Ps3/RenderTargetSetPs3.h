#ifndef traktor_render_RenderTargetSetPs3_H
#define traktor_render_RenderTargetSetPs3_H

#include "Core/RefArray.h"
#include "Render/RenderTargetSet.h"
#include "Render/Types.h"
#include "Render/Ps3/PlatformPs3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_PS3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class LocalMemoryObject;
class RenderTargetPs3;

class T_DLLCLASS RenderTargetSetPs3 : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	RenderTargetSetPs3();

	virtual ~RenderTargetSetPs3();

	bool create(const RenderTargetSetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual Ref< ITexture > getColorTexture(int index) const;

	virtual void swap(int index1, int index2);

	virtual bool read(int index, void* buffer) const;

	const CellGcmTexture& getGcmDepthTexture();

	RenderTargetPs3* getRenderTarget(int index) {
		return m_renderTargets[index];
	}

private:
	int32_t m_width;
	int32_t m_height;
	RefArray< RenderTargetPs3 > m_renderTargets;
	CellGcmTexture m_depthTexture;
	LocalMemoryObject* m_depthData;
};

	}
}

#endif	// traktor_render_RenderTargetSetPs3_H
