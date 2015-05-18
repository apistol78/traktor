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

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

	Ref< AbstractSampler > createSampler() const;

	inline const uint8_t* getData() const { return m_data; }

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
