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
	namespace db
	{

class Database;

	}

	namespace flash
	{

class FlashMovie;

class T_DLLCLASS FlashMovieLoader : public IFlashMovieLoader
{
	T_RTTI_CLASS;

public:
	FlashMovieLoader(db::Database* database);

	virtual Ref< FlashMovie > load(const std::wstring& name) const T_OVERRIDE;

private:
	Ref< db::Database > m_database;
};

	}
}

#endif	// traktor_flash_FlashMovieLoader
