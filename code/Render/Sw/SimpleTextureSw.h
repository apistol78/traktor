#pragma once

#include "Render/ISimpleTexture.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{
		
class AbstractSampler;

/*!
 * \ingroup SW
 */
class SimpleTextureSw : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	SimpleTextureSw();

	virtual ~SimpleTextureSw();
	
	bool create(const SimpleTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getWidth() const override final;
	
	virtual int32_t getHeight() const override final;

	virtual int32_t getMips() const override final;

	virtual bool lock(int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t level) override final;

	virtual void* getInternalHandle() override final;

	Ref< AbstractSampler > createSampler() const;

	const uint8_t* getData() const { return m_data; }

private:
	int32_t m_width;
	int32_t m_height;
	TextureFormat m_format;
	uint8_t* m_data;
	uint8_t* m_lock;
};
		
	}
}
