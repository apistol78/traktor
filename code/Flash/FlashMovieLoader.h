#ifndef traktor_flash_FlashMovieLoader
#define traktor_flash_FlashMovieLoader

#include "Flash/IFlashMovieLoader.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Job;

	namespace flash
	{

class T_DLLCLASS FlashMovieLoader : public IFlashMovieLoader
{
	T_RTTI_CLASS;

public:
	FlashMovieLoader(bool merge);

	virtual Ref< IHandle > loadAsync(const net::Url& url) const T_OVERRIDE T_FINAL;

	virtual Ref< FlashMovie > load(const net::Url& url) const T_OVERRIDE T_FINAL;

private:
	bool m_merge;
};

	}
}

#endif	// traktor_flash_FlashMovieLoader
