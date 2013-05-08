#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Editor/App/InstanceClipboardData.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.InstanceClipboardData", 0, InstanceClipboardData, ISerializable)

InstanceClipboardData::InstanceClipboardData()
{
}

InstanceClipboardData::InstanceClipboardData(const std::wstring& name, ISerializable* object)
:	m_name(name)
,	m_object(object)
{
}

const std::wstring& InstanceClipboardData::getName() const
{
	return m_name;
}

ISerializable* InstanceClipboardData::getObject() const
{
	return m_object;
}

void InstanceClipboardData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberRef< ISerializable >(L"object", m_object);
}

	}
}
