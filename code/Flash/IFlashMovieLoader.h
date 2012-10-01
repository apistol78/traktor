#ifndef traktor_flash_IFlashMovieLoader
#define traktor_flash_IFlashMovieLoader

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

class T_DLLCLASS IFlashMovieLoader : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< FlashMovie > load(const std::wstring& name) const = 0;
};

	}
}

#endif	// traktor_flash_IFlashMovieLoader
