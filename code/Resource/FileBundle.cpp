#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Resource/FileBundle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.resource.FileBundle", 0, FileBundle, ISerializable)

std::wstring FileBundle::lookup(const std::wstring& id) const
{
	auto it = m_dataIds.find(toLower(id));
	return it != m_dataIds.end() ? it->second : L"";
}

void FileBundle::serialize(ISerializer& s)
{
	s >> MemberStlMap< std::wstring, std::wstring >(L"dataIds", m_dataIds);
}

	}
}
