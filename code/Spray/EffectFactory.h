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
	EffectFactory(const world::IEntityBuilder* entityBuilder);

	virtual const TypeInfoSet getResourceTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const T_OVERRIDE T_FINAL;

	virtual bool isCacheable(const TypeInfo& productType) const T_OVERRIDE T_FINAL;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const T_OVERRIDE T_FINAL;

private:
	Ref< const world::IEntityBuilder > m_entityBuilder;
};

	}
}

#endif	// traktor_spray_EffectFactory_H
