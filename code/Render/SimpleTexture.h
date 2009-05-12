#ifndef traktor_render_SimpleTexture_H
#define traktor_render_SimpleTexture_H

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

/*! \brief Simple 2d texture.
 * \ingroup Render
 */
class T_DLLCLASS SimpleTexture : public Texture
{
	T_RTTI_CLASS(SimpleTexture)
	
public:
	virtual bool lock(int level, Lock& lock) = 0;

	virtual void unlock(int level) = 0;
};

	}
}

#endif	// traktor_render_SimpleTexture_H
