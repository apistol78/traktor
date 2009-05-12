#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberStl.h>
#include <Core/Serialization/MemberComposite.h>
#include "App/UpdateBundle.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.drone.UpdateBundle", UpdateBundle, Serializable)

UpdateBundle::UpdateBundle()
:	m_version(0)
{
}

const std::wstring& UpdateBundle::getDescription() const
{
	return m_description;
}

uint32_t UpdateBundle::getBundleVersion() const
{
	return m_version;
}

const std::vector< UpdateBundle::BundledItem >& UpdateBundle::getItems() const
{
	return m_items;
}

bool UpdateBundle::serialize(Serializer& s)
{
	s >> Member< uint32_t >(L"version", m_version);
	s >> Member< std::wstring >(L"description", m_description);
	s >> MemberStlVector< BundledItem, MemberComposite< BundledItem > >(L"items", m_items);
	return true;
}

bool UpdateBundle::BundledItem::serialize(Serializer& s)
{
	s >> Member< std::wstring >(L"url", url);
	s >> Member< std::wstring >(L"path", path);
	return true;
}

	}
}
