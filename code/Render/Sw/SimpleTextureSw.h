/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_SimpleTextureSw_H
#define traktor_render_SimpleTextureSw_H

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

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;
	
	virtual bool lock(int level, Lock& lock) override final;

	virtual void unlock(int level) override final;

	virtual void* getInternalHandle() override final;

	Ref< AbstractSampler > createSampler() const;

	const uint8_t* getData() const { return m_data; }

private:
	int m_width;
	int m_height;
	TextureFormat m_format;
	uint8_t* m_data;
	uint8_t* m_lock;
};
		
	}
}

#endif	// traktor_render_SimpleTextureSw_H
