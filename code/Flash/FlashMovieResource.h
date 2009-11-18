#ifndef traktor_flash_FlashMovieResource_H
#define traktor_flash_FlashMovieResource_H

#include <string>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief Flash movie resource.
 * \ingroup Flash
 */
class T_DLLCLASS FlashMovieResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool serialize(ISerializer& s);
};

	}
}

#endif	// traktor_flash_FlashMovieResource_H
