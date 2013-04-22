#ifndef traktor_flash_FlashMovieResourceFactory_H
#define traktor_flash_FlashMovieResourceFactory_H

#include "Resource/IResourceFactory.h"

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

/*! \brief Flash movie resource factory.
 * \ingroup Flash
 */
class T_DLLCLASS FlashMovieResourceFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	FlashMovieResourceFactory(db::Database* db);

	virtual const TypeInfoSet getResourceTypes() const;

	virtual const TypeInfoSet getProductTypes() const;

	virtual bool isCacheable() const;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid) const;

private:
	Ref< db::Database > m_db;
};

	}
}

#endif	// traktor_flash_FlashMovieResourceFactory_H
