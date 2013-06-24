#ifndef DependencyCache_H
#define DependencyCache_H

#include <map>
#include <Core/Serialization/ISerializable.h>
#include <Core/Misc/MD5.h>

class DependencyCache : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	void set(const std::wstring& key, const traktor::MD5& md5, const std::set< std::wstring >& dependencies);

	bool get(const std::wstring& key, const traktor::MD5& md5, std::set< std::wstring >& outDependencies) const;

	virtual void serialize(traktor::ISerializer& s);

private:
	struct CacheItem
	{
		uint32_t md5[4];
		std::set< std::wstring > dependencies;

		void serialize(traktor::ISerializer& s);
	};

	std::map< std::wstring, CacheItem > m_cache;
};

#endif	// DependencyCache_H
