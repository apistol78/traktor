#ifndef traktor_render_VolumeTextureDx10_H
#define traktor_render_VolumeTextureDx10_H

#include "Render/IVolumeTexture.h"

namespace traktor
{
	namespace render
	{
		
/*!
 * \ingroup DX10
 */
class VolumeTextureDx10 : public IVolumeTexture
{
	T_RTTI_CLASS;
	
public:
	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;
};
		
	}
}

#endif	// traktor_render_VolumeTextureDx10_H
