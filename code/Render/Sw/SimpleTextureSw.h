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

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual bool lock(int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int level) T_OVERRIDE T_FINAL;

	virtual void* getInternalHandle() T_OVERRIDE T_FINAL;

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
