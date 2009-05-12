#ifndef DependencyCache_H
#define DependencyCache_H

#include <map>
#include <Core/Serialization/Serializable.h>
#include <Core/Misc/MD5.h>

class DependencyCache : public traktor::Serializable
{
	T_RTTI_CLASS(DependencyCache)

public:
	void set(const std::wstring& key, const traktor::MD5& md5, const std::set< std::wstring >& dependencies);

	bool get(const std::wstring& key, const traktor::MD5& md5, std::set< std::wstring >& outDependencies) const;

	virtual bool serialize(traktor::Serializer& s);

private:
	struct CacheItem
	{
		uint32_t md5[4];
		std::set< std::wstring > dependencies;

		bool serialize(traktor::Serializer& s);
	};

	std::map< std::wstring, CacheItem > m_cache;
};

#endif	// DependencyCache_H
