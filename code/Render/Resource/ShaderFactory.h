#ifndef traktor_render_ShaderFactory_H
#define traktor_render_ShaderFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
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

	namespace render
	{

class IRenderSystem;

/*! \brief Shader resource factory.
 * \ingroup Render
 */
class T_DLLCLASS ShaderFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	ShaderFactory(
		db::Database* database,
		IRenderSystem* renderSystem
	);

	virtual const TypeInfoSet getResourceTypes() const;

	virtual const TypeInfoSet getProductTypes() const;

	virtual bool isCacheable() const;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const;

private:
	Ref< db::Database > m_database;
	Ref< IRenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_render_ShaderFactory_H
