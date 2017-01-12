#ifndef traktor_spark_FontResourceFactory_H
#define traktor_spark_FontResourceFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;

	}

	namespace spark
	{

/*! \brief Font resource factory.
 * \ingroup Spark
 */
class T_DLLCLASS FontResourceFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	FontResourceFactory(render::IRenderSystem* renderSystem);

	virtual const TypeInfoSet getResourceTypes() const T_FINAL;

	virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const T_OVERRIDE T_FINAL;

	virtual bool isCacheable(const TypeInfo& productType) const T_FINAL;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const T_FINAL;

private:
	Ref< render::IRenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_spark_FontResourceFactory_H
