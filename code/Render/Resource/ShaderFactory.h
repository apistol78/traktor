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
	ShaderFactory(IRenderSystem* renderSystem);

	virtual const TypeInfoSet getResourceTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const T_OVERRIDE T_FINAL;

	virtual bool isCacheable(const TypeInfo& productType) const T_OVERRIDE T_FINAL;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const T_OVERRIDE T_FINAL;

private:
	Ref< IRenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_render_ShaderFactory_H
