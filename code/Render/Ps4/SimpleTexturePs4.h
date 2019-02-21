#pragma once

#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace render
	{

class ContextPs4;
struct SimpleTextureCreateDesc;
		
/*!
 * \ingroup GNM
 */
class SimpleTexturePs4 : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	SimpleTexturePs4(ContextPs4* context);

	virtual ~SimpleTexturePs4();

	bool create(const SimpleTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getWidth() const override final;
	
	virtual int32_t getHeight() const override final;

	virtual bool lock(int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t level) override final;

	virtual void* getInternalHandle() override final;

private:
	Ref< ContextPs4 > m_context;
};
		
	}
}
