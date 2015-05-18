#ifndef traktor_render_RenderTargetSw_H
#define traktor_render_RenderTargetSw_H

#include "Core/Misc/AutoPtr.h"
#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace render
	{

class AbstractSampler;

struct RenderTargetCreateDesc;
struct RenderTargetSetCreateDesc;

/*!
 * \ingroup SW
 */
class RenderTargetSw : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	RenderTargetSw();

	bool create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc);

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

	uint32_t* getColorSurface();

	Ref< AbstractSampler > createSampler() const;

private:
	AutoArrayPtr< uint32_t > m_color;
	int m_width;
	int m_height;
};

	}
}

#endif	// traktor_render_RenderTargetSw_H
