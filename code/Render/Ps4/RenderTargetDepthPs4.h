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

/*!
 * \ingroup GNM
 */
class RenderTargetDepthPs4 : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	RenderTargetDepthPs4(ContextPs4* context);

	virtual ~RenderTargetDepthPs4();

	bool create(const RenderTargetSetCreateDesc& setDesc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int32_t getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int32_t getHeight() const T_OVERRIDE T_FINAL;

	virtual int32_t getMips() const T_OVERRIDE T_FINAL;

	virtual bool lock(int32_t level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int32_t level) T_OVERRIDE T_FINAL;

	virtual void* getInternalHandle() T_OVERRIDE T_FINAL;

	const sce::Gnm::DepthRenderTarget& getDepthTargetGnm() const { return m_depthTarget; }

private:
	Ref< ContextPs4 > m_context;
	sce::Gnm::DepthRenderTarget m_depthTarget;
	Ref< MemoryHeapObjectPs4 > m_memoryHTile;
	Ref< MemoryHeapObjectPs4 > m_memoryStencil;
	Ref< MemoryHeapObjectPs4 > m_memory;
	int32_t m_width;
	int32_t m_height;
};

	}
}
