#ifndef traktor_render_RenderTargetPs3_H
#define traktor_render_RenderTargetPs3_H

#include <cell/gcm.h>
#include "Render/RenderTarget.h"
#include "Render/Types.h"
#include "Render/Ps3/TypesPs3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_PS3_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class T_DLLCLASS RenderTargetPs3 : public RenderTarget
{
	T_RTTI_CLASS(RenderTargetPs3)

public:
	RenderTargetPs3();

	bool create(const RenderTargetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	void bind(int stage, const SamplerState& samplerState);

	inline uint32_t getGcmSurfaceColorFormat() const
	{
		return m_colorSurfaceFormat;
	}

	inline const CellGcmTexture& getGcmColorTexture() const
	{
		return m_colorTexture;
	}

	inline const CellGcmTexture& getGcmDepthTexture() const
	{
		return m_depthTexture;
	}

private:
	uint32_t m_colorSurfaceFormat;
	CellGcmTexture m_colorTexture;
	CellGcmTexture m_depthTexture;
	void* m_colorData;
	void* m_depthData;
	int m_width;
	int m_height;
};

	}
}

#endif	// traktor_render_RenderTargetPs3_H
