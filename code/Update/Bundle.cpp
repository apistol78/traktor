#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Update/Bundle.h"
#include "Update/IPostAction.h"
#include "Update/Item.h"

namespace traktor
{
	namespace update
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.update.Bundle", 0, Bundle, ISerializable)

Bundle::Bundle()
:	m_version(0)
{
}

int32_t Bundle::getVersion() const
{
	return m_version;
}

const std::wstring& Bundle::getDescription() const
{
	return m_description;
}

const RefArray< Item >& Bundle::getItems() const
{
	return m_items;
}

const RefArray< IPostAction >& Bundle::getPostActions() const
{
	return m_postActions;
}

bool Bundle::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"version", m_version);
	s >> Member< std::wstring >(L"description", m_description);
	s >> MemberRefArray< Item >(L"items", m_items);
	s >> MemberRefArray< IPostAction >(L"postActions", m_postActions);
	return true;
}

	}
}
