#ifndef traktor_flash_FlashMovieFactory_H
#define traktor_flash_FlashMovieFactory_H

#include <map>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class SwfReader;
class FlashMovie;
class FlashTag;

/*! \brief Flash movie factory.
 * \ingroup Flash
 */
class T_DLLCLASS FlashMovieFactory : public Object
{
public:
	FlashMovieFactory(bool allowNPOT);

	Ref< FlashMovie > createMovie(SwfReader* swf);

private:
	bool m_allowNPOT;
	std::map< uint16_t, Ref< FlashTag > > m_tagReaders;
};

	}
}

#endif	// traktor_flash_FlashMovieFactory_H
