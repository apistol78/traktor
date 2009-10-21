#ifndef traktor_weather_CloudMaskFactory_H
#define traktor_weather_CloudMaskFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
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

	namespace weather
	{

class T_DLLCLASS CloudMaskFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS(CloudMaskFactory)

public:
	CloudMaskFactory(db::Database* db);

	virtual const TypeSet getResourceTypes() const;

	virtual bool isCacheable() const;

	virtual Object* create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid);

private:
	Ref< db::Database > m_db;
};

	}
}

#endif	// traktor_weather_CloudMaskFactory_H
