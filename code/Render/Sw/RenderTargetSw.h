#pragma once

#include "Core/Ref.h"
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

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getWidth() const override final;
	
	virtual int32_t getHeight() const override final;

	virtual int32_t getMips() const override final;

	virtual bool lock(int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t level) override final;

	virtual void* getInternalHandle() override final;

	uint32_t* getColorSurface();

	Ref< AbstractSampler > createSampler() const;

private:
	AutoArrayPtr< uint32_t > m_color;
	int32_t m_width;
	int32_t m_height;
};

	}
}
