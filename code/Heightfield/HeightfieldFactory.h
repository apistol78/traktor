#ifndef traktor_hf_HeightfieldFactory_H
#define traktor_hf_HeightfieldFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EXPORT)
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

	namespace hf
	{

class T_DLLCLASS HeightfieldFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	HeightfieldFactory(db::Database* database);

	virtual const TypeInfoSet getResourceTypes() const;

	virtual const TypeInfoSet getProductTypes() const;

	virtual bool isCacheable() const;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid) const;

private:
	Ref< db::Database > m_database;
};

	}
}

#endif	// traktor_hf_HeightfieldFactory_H
