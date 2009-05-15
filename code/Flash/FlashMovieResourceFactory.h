#ifndef traktor_flash_FlashMovieResourceFactory_H
#define traktor_flash_FlashMovieResourceFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/ResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace flash
	{

/*! \brief Flash movie resource factory.
 * \ingroup Flash
 */
class T_DLLCLASS FlashMovieResourceFactory : public resource::ResourceFactory
{
	T_RTTI_CLASS(FlashMovieResourceFactory)

public:
	FlashMovieResourceFactory(db::Database* db);

	virtual const TypeSet getResourceTypes() const;

	virtual Object* create(const Type& resourceType, const Guid& guid, bool& outCacheable);

private:
	Ref< db::Database > m_db;
};

	}
}

#endif	// traktor_flash_FlashMovieResourceFactory_H
