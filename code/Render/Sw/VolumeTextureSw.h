#ifndef traktor_render_VolumeTextureSw_H
#define traktor_render_VolumeTextureSw_H

#include "Render/IVolumeTexture.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{
		
/*!
 * \ingroup SW
 */
class VolumeTextureSw : public IVolumeTexture
{
	T_RTTI_CLASS;

public:
	VolumeTextureSw();

	virtual ~VolumeTextureSw();
	
	bool create(const VolumeTextureCreateDesc& desc);

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	inline const uint32_t* getData() const { return m_data; }

private:
	int m_width;
	int m_height;
	int m_depth;
	TextureFormat m_format;
	uint32_t* m_data;
};
		
	}
}

#endif	// traktor_render_VolumeTextureSw_H
