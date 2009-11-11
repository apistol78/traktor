#ifndef traktor_world_PostProcessFactory_H
#define traktor_world_PostProcessFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
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

	namespace world
	{

/*! \brief Post processing resource factory.
 * \ingroup World
 */
class T_DLLCLASS PostProcessFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS(PostProcessFactory)

public:
	PostProcessFactory(db::Database* db);

	virtual const TypeSet getResourceTypes() const;

	virtual bool isCacheable() const;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid);

private:
	Ref< db::Database > m_db;
};

	}
}

#endif	// traktor_world_PostProcessFactory_H
