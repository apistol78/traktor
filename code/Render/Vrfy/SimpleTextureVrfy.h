#pragma once

#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vrfy
 */
class SimpleTextureVrfy : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	SimpleTextureVrfy(ISimpleTexture* texture);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual bool lock(int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t level) override final;

	virtual void* getInternalHandle() override final;

	ISimpleTexture* getTexture() const { return m_texture; }

private:
	Ref< ISimpleTexture > m_texture;
	int32_t m_locked;
};

	}
}
