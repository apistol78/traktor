#ifndef traktor_physics_MeshFactory_H
#define traktor_physics_MeshFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
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

	namespace physics
	{

/*! \brief Mesh geometry resource factory.
 * \ingroup Physics
 */
class T_DLLCLASS MeshFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	MeshFactory(db::Database* db);

	virtual const TypeInfoSet getResourceTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getProductTypes() const T_OVERRIDE T_FINAL;

	virtual bool isCacheable() const T_OVERRIDE T_FINAL;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const T_OVERRIDE T_FINAL;

private:
	Ref< db::Database > m_db;
};

	}
}

#endif	// traktor_physics_MeshFactory_H
