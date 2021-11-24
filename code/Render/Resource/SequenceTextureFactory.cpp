#include "Database/Instance.h"
#include "Render/SequenceTexture.h"
#include "Render/Resource/SequenceTextureResource.h"
#include "Render/Resource/SequenceTextureFactory.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SequenceTextureFactory", SequenceTextureFactory, resource::IResourceFactory)

const TypeInfoSet SequenceTextureFactory::getResourceTypes() const
{
	return makeTypeInfoSet< SequenceTextureResource >();
}

const TypeInfoSet SequenceTextureFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< ITexture >();
}

bool SequenceTextureFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > SequenceTextureFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< SequenceTextureResource > resource = instance->getObject< SequenceTextureResource >();
	if (!resource)
		return nullptr;

	Ref< SequenceTexture > sequenceTexture = new SequenceTexture();
	sequenceTexture->m_rate = resource->m_rate;

	for (const auto& rt : resource->m_textures)
	{
		resource::Proxy< ITexture > texture;
		if (resourceManager->bind(rt, texture))
			sequenceTexture->m_textures.push_back(texture);
		else
			return nullptr;
	}

	return sequenceTexture;
}

	}
}
