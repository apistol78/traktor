#ifndef traktor_spray_EffectFactory_H
#define traktor_spray_EffectFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
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

	namespace spray
	{

/*! \brief Emitter resource factory.
 * \ingroup Spray
 */
class T_DLLCLASS EffectFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS(EffectFactory)

public:
	EffectFactory(db::Database* db);

	virtual const TypeSet getResourceTypes() const;

	virtual bool isCacheable() const;

	virtual Object* create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid);

private:
	Ref< db::Database > m_db;
};

	}
}

#endif	// traktor_spray_EffectFactory_H
