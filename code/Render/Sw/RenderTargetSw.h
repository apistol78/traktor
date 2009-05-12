#ifndef traktor_render_RenderTargetSw_H
#define traktor_render_RenderTargetSw_H

#include <vector>
#include "Render/Texture.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_SW_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

struct RenderTargetSetCreateDesc;
struct RenderTargetCreateDesc;

/*!
 * \ingroup SW
 */
class T_DLLCLASS RenderTargetSw : public Texture
{
	T_RTTI_CLASS(RenderTargetSw)

public:
	RenderTargetSw();

	bool create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	uint32_t* getColorSurface();

private:
	std::vector< uint32_t > m_color;
	int m_width;
	int m_height;
};

	}
}

#endif	// traktor_render_RenderTargetSw_H
