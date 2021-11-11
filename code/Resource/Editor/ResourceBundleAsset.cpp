#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Resource/Editor/ResourceBundleAsset.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.resource.ResourceBundleAsset", 0, ResourceBundleAsset, ISerializable)

void ResourceBundleAsset::add(const Guid& resource)
{
	m_resources.push_back(resource);
}

const AlignedVector< Guid >& ResourceBundleAsset::get() const
{
	return m_resources;
}

void ResourceBundleAsset::setPersistent(bool persistent)
{
	m_persistent = persistent;
}

bool ResourceBundleAsset::persistent() const
{
	return m_persistent;
}

void ResourceBundleAsset::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Guid >(L"resources", m_resources);
	s >> Member< bool >(L"persistent", m_persistent);
}

	}
}
