#ifndef traktor_render_RenderTargetPs3_H
#define traktor_render_RenderTargetPs3_H

#include "Render/ITexture.h"
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

class T_DLLCLASS RenderTargetPs3 : public ITexture
{
	T_RTTI_CLASS;

public:
	RenderTargetPs3();

	bool create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	void bind(int stage, const SamplerState& samplerState);

	uint32_t getGcmSurfaceColorFormat() const {
		return m_colorSurfaceFormat;
	}

	const CellGcmTexture& getGcmColorTexture() const {
		return m_colorTexture;
	}

private:
	int32_t m_width;
	int32_t m_height;
	uint32_t m_colorSurfaceFormat;
	CellGcmTexture m_colorTexture;
	void* m_colorData;
};

	}
}

#endif	// traktor_render_RenderTargetPs3_H
