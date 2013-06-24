#include <Core/Log/Log.h>
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/MemberStl.h>
#include <Core/Serialization/MemberComposite.h>
#include <Core/Serialization/MemberStaticArray.h>
#include "SolutionBuilderLIB/Make/DependencyCache.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"DependencyCache", 0, DependencyCache, ISerializable)

void DependencyCache::set(const std::wstring& key, const MD5& md5, const std::set< std::wstring >& dependencies)
{
	CacheItem item;
	item.md5[0] = md5.get()[0];
	item.md5[1] = md5.get()[1];
	item.md5[2] = md5.get()[2];
	item.md5[3] = md5.get()[3];
	item.dependencies = dependencies;
	m_cache[key] = item;
}

bool DependencyCache::get(const std::wstring& key, const MD5& md5, std::set< std::wstring >& outDependencies) const
{
	std::map< std::wstring, CacheItem >::const_iterator i = m_cache.find(key);
	if (i == m_cache.end())
		return false;

	if (memcmp(i->second.md5, md5.get(), 4 * sizeof(uint32_t)) != 0)
	{
		traktor::log::info << L"File \"" << key << L"\" modified, cached dependencies invalid" << Endl;
		return false;
	}

	outDependencies = i->second.dependencies;
	return true;
}

void DependencyCache::serialize(ISerializer& s)
{
	s >> MemberStlMap< std::wstring, CacheItem, MemberStlPair< std::wstring, CacheItem, Member< std::wstring >, MemberComposite< CacheItem > > >(L"cache", m_cache);
}

void DependencyCache::CacheItem::serialize(ISerializer& s)
{
	s >> MemberStaticArray< uint32_t, 4 >(L"md5", md5);
	s >> MemberStlSet< std::wstring >(L"dependencies", dependencies);
}
