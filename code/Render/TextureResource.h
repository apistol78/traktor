#ifndef traktor_render_TextureResource_H
#define traktor_render_TextureResource_H

#include "Core/Serialization/Serializable.h"

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

/*! \brief Texture resource.
 * \ingroup Render
 */
class T_DLLCLASS TextureResource : public Serializable
{
	T_RTTI_CLASS(TextureResource)

public:
	virtual bool serialize(Serializer& s);
};

	}
}

#endif	// traktor_render_TextureResource_H
