#include "Database/Instance.h"
#include "Render/Resource/AliasTextureResource.h"
#include "Render/Resource/AliasTextureFactory.h"
#include "Render/Resource/TextureProxy.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.AliasTextureFactory", AliasTextureFactory, resource::IResourceFactory)

const TypeInfoSet AliasTextureFactory::getResourceTypes() const
{
	return makeTypeInfoSet< AliasTextureResource >();
}

const TypeInfoSet AliasTextureFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< ITexture >();
}

bool AliasTextureFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > AliasTextureFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< AliasTextureResource > resource = instance->getObject< AliasTextureResource >();
	if (!resource)
		return nullptr;

	resource::Proxy< ITexture > texture;
	if (!resourceManager->bind(resource->m_texture, texture))
		return false;

	return new TextureProxy(texture);
}

	}
}
