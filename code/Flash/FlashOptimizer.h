#ifndef traktor_flash_FlashOptimizer_H
#define traktor_flash_FlashOptimizer_H

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

class FlashMovie;

/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS FlashOptimizer : public Object
{
	T_RTTI_CLASS;

public:
	Ref< FlashMovie > optimizeStaticMovie(const FlashMovie* movie) const;
};

	}
}

#endif	// traktor_flash_FlashOptimizer_H
