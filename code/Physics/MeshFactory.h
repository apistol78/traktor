#ifndef traktor_physics_MeshFactory_H
#define traktor_physics_MeshFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
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

	namespace physics
	{

/*! \brief Mesh geometry resource factory.
 * \ingroup Physics
 */
class T_DLLCLASS MeshFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS(MeshFactory)

public:
	MeshFactory(db::Database* db);

	virtual const TypeSet getResourceTypes() const;

	virtual bool isCacheable() const;

	virtual Object* create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid);

private:
	Ref< db::Database > m_db;
};

	}
}

#endif	// traktor_physics_MeshFactory_H
