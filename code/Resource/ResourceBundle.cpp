#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberType.h"
#include "Resource/ResourceBundle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.resource.ResourceBundle", 0, ResourceBundle, ISerializable)

ResourceBundle::ResourceBundle()
:	m_persistent(false)
{
}

ResourceBundle::ResourceBundle(const std::vector< std::pair< const TypeInfo*, Guid > >& resources, bool persistent)
:	m_resources(resources)
,	m_persistent(persistent)
{
}

const std::vector< std::pair< const TypeInfo*, Guid > >& ResourceBundle::get() const
{
	return m_resources;
}

bool ResourceBundle::persistent() const
{
	return m_persistent;
}

void ResourceBundle::serialize(ISerializer& s)
{
	s >> MemberStlVector< std::pair< const TypeInfo*, Guid >, MemberStlPair< const TypeInfo*, Guid, MemberType, Member< Guid > > >(L"resources", m_resources);
	s >> Member< bool >(L"persistent", m_persistent);
}

	}
}
