#include "Database/Database.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashMovieLoader.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMovieLoader", FlashMovieLoader, IFlashMovieLoader)

FlashMovieLoader::FlashMovieLoader(db::Database* database)
:	m_database(database)
{
}

Ref< FlashMovie > FlashMovieLoader::load(const std::wstring& name) const
{
	Guid movieId(name);
	if (movieId.isNotNull())
		return m_database->getObjectReadOnly< FlashMovie >(movieId);
	else
		return 0;
}

	}
}
