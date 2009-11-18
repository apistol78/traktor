#ifndef traktor_render_IVolumeTexture_H
#define traktor_render_IVolumeTexture_H

#include "Render/ITexture.h"

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
class T_DLLCLASS IVolumeTexture : public ITexture
{
	T_RTTI_CLASS;
};
	
	}
}

#endif	// traktor_render_IVolumeTexture_H
