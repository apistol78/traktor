#ifndef traktor_flash_IFlashMovieLoader
#define traktor_flash_IFlashMovieLoader

#include "Core/Object.h"
#include "Net/Url.h"

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
	class IHandle : public Object
	{
		T_RTTI_CLASS;

	public:
		virtual bool wait() = 0;

		virtual bool ready() = 0;

		virtual bool succeeded() = 0;

		virtual Ref< FlashMovie > get() = 0;

		bool failed() { return !succeeded(); }
	};

	virtual Ref< IHandle > loadAsync(const net::Url& url) const = 0;

	virtual Ref< FlashMovie > load(const net::Url& url) const = 0;
};

	}
}

#endif	// traktor_flash_IFlashMovieLoader
