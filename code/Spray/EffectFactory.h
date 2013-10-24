#ifndef traktor_spray_EffectFactory_H
#define traktor_spray_EffectFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
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

	namespace world
	{

class IEntityBuilder;

	}

	namespace spray
	{

/*! \brief Emitter resource factory.
 * \ingroup Spray
 */
class T_DLLCLASS EffectFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	EffectFactory(db::Database* db, const world::IEntityBuilder* entityBuilder);

	virtual const TypeInfoSet getResourceTypes() const;

	virtual const TypeInfoSet getProductTypes() const;

	virtual bool isCacheable() const;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const;

private:
	Ref< db::Database > m_db;
	Ref< const world::IEntityBuilder > m_entityBuilder;
};

	}
}

#endif	// traktor_spray_EffectFactory_H
