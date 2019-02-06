#pragma once

#include <gnm.h>
#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace render
	{

class ContextPs4;
class MemoryHeapObjectPs4;
struct RenderTargetSetCreateDesc;
struct RenderTargetCreateDesc;

/*!
 * \ingroup GNM
 */
class RenderTargetPs4 : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	RenderTargetPs4(ContextPs4* context);

	virtual ~RenderTargetPs4();

	bool create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getWidth() const override final;
	
	virtual int32_t getHeight() const override final;

	virtual int32_t getMips() const override final;

	virtual bool lock(int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t level) override final;

	virtual void* getInternalHandle() override final;

	const sce::Gnm::RenderTarget& getRenderTargetGnm() const { return m_renderTarget; }

private:
	Ref< ContextPs4 > m_context;
	sce::Gnm::RenderTarget m_renderTarget;
	Ref< MemoryHeapObjectPs4 > m_memory;
	int32_t m_width;
	int32_t m_height;
};

	}
}
