#ifndef traktor_render_VolumeTextureDx10_H
#define traktor_render_VolumeTextureDx10_H

#include "Render/VolumeTexture.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX10_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{
		
/*!
 * \ingroup DX10
 */
class T_DLLCLASS VolumeTextureDx10 : public VolumeTexture
{
	T_RTTI_CLASS(VolumeTextureDx10)
	
public:
	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;
};
		
	}
}

#endif	// traktor_render_VolumeTextureDx10_H
