#ifndef traktor_render_VolumeTexture_H
#define traktor_render_VolumeTexture_H

#include "Render/Texture.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Volume texture.
 * \ingroup Render
 */
class T_DLLCLASS VolumeTexture : public Texture
{
	T_RTTI_CLASS(VolumeTexture)
};
	
	}
}

#endif	// traktor_render_VolumeTexture_H
