#include <algorithm>
#include "Resource/ResourceLoader.h"
#include "Resource/ResourceFactory.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ResourceLoader", ResourceLoader, Object)

void ResourceLoader::addFactory(ResourceFactory* factory)
{
	m_factories.push_back(factory);
}

Object* ResourceLoader::load(const Type& type, const Guid& guid)
{
	Ref< ResourceFactory > factory;

	for (RefArray< ResourceFactory >::iterator i = m_factories.begin(); i != m_factories.end(); ++i)
	{
		TypeSet typeSet = (*i)->getResourceTypes();
		if (std::find(typeSet.begin(), typeSet.end(), &type) != typeSet.end())
		{
			factory = *i;
			break;
		}
	}

	if (!factory)
	{
		log::error << L"Unable to find resource factory for type \"" << type.getName() << L"\"" << Endl;
		return 0;
	}

	Ref< Object > resource = factory->create(type, guid);
	if (!resource)
	{
		log::error << L"Unable to create resource \"" << guid.format() << L"\" (" << type.getName() << L")" << Endl;
		return 0;
	}

	T_ASSERT_M (is_type_of(type, resource->getType()), L"Incorrect type of created resource");
	log::info << L"Resource \"" << guid.format() << L"\" (" << type_name(resource) << L") loaded" << Endl;
		
	return resource;
}

	}
}
