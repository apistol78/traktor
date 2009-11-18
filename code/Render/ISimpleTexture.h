#ifndef traktor_render_ISimpleTexture_H
#define traktor_render_ISimpleTexture_H

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

/*! \brief Simple 2d texture.
 * \ingroup Render
 */
class T_DLLCLASS ISimpleTexture : public ITexture
{
	T_RTTI_CLASS;
	
public:
	virtual bool lock(int level, Lock& lock) = 0;

	virtual void unlock(int level) = 0;
};

	}
}

#endif	// traktor_render_ISimpleTexture_H
