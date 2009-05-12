#ifndef traktor_render_CubeTexture_H
#define traktor_render_CubeTexture_H

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

/*! \brief Cube texture.
 * \ingroup Render
 */
class T_DLLCLASS CubeTexture : public Texture
{
	T_RTTI_CLASS(CubeTexture)
	
public:
	enum Side
	{
		SdPositiveX	= 0,
		SdNegativeX = 1,
		SdPositiveY = 2,
		SdNegativeY = 3,
		SdPositiveZ = 4,
		SdNegativeZ = 5
	};

	virtual bool lock(int side, int level, Lock& lock) = 0;

	virtual void unlock(int side, int level) = 0;
};
	
	}
}

#endif	// traktor_render_CubeTexture_H
