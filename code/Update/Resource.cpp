#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Update/Resource.h"

namespace traktor
{
	namespace update
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.update.Resource", 0, Resource, Item)

Resource::Resource()
:	m_size(0)
,	m_autoLaunch(false)
{
}

uint32_t Resource::getSize() const
{
	return m_size;
}

const MD5& Resource::getMD5() const
{
	return m_md5;
}

const std::wstring& Resource::getUrl() const
{
	return m_url;
}

const std::wstring& Resource::getTargetPath() const
{
	return m_targetPath;
}

bool Resource::autoLaunch() const
{
	return m_autoLaunch;
}

bool Resource::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"size", m_size);
	s >> MemberComposite< MD5, false >(L"md5", m_md5);
	s >> Member< std::wstring >(L"url", m_url);
	s >> Member< std::wstring >(L"targetPath", m_targetPath);
	s >> Member< bool >(L"autoLaunch", m_autoLaunch);
	return true;
}

	}
}
