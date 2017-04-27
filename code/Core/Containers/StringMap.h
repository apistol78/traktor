/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_StringMap_H
#define traktor_StringMap_H

#include "Core/Containers/SmallMap.h"

namespace traktor
{

/*! \brief String map
 * \ingroup Core
 */
template < typename ValueType, typename StringType = std::string, int BucketCount = 128 >
class StringMap
{
public:
	typedef ValueType value_t;
	typedef StringType string_t;
	typedef typename StringType::const_pointer const_pointer_t;

	value_t* find(const string_t& str)
	{
		uint32_t hash = shash(str) & (BucketCount - 1);
		SmallMap< string_t, value_t >& buckets = m_buckets[hash & (BucketCount - 1)];
		typename SmallMap< string_t, value_t >::iterator i = buckets.find(str);
		return i != buckets.end() ? &i->second : 0;
	}

	const value_t* find(const string_t& str) const
	{
		uint32_t hash = shash(str) & (BucketCount - 1);
		const SmallMap< string_t, value_t >& buckets = m_buckets[hash & (BucketCount - 1)];
		typename SmallMap< string_t, value_t >::const_iterator i = buckets.find(str);
		return i != buckets.end() ? &i->second : 0;
	}

	void insert(const string_t& str, const value_t& value)
	{
		uint32_t hash = shash(str) & (BucketCount - 1);
		m_buckets[hash].insert(str, value);
	}

	bool remove(const string_t& str)
	{
		uint32_t hash = shash(str) & (BucketCount - 1);
		SmallMap< string_t, value_t >& buckets = m_buckets[hash & (BucketCount - 1)];
		typename SmallMap< string_t, value_t >::iterator i = buckets.find(str);
		if (i == buckets.end())
			return false;
		buckets.erase(i);
		return true;
	}

	void clear()
	{
		for (int32_t i = 0; i < BucketCount; ++i)
			m_buckets[i].clear();
	}

	value_t& operator [] (const string_t& str)
	{
		uint32_t hash = shash(str) & (BucketCount - 1);
		return m_buckets[hash][str];
	}

	const value_t& operator [] (const string_t& str) const
	{
		uint32_t hash = shash(str) & (BucketCount - 1);
		return m_buckets[hash][str];
	}

private:
	SmallMap< string_t, value_t > m_buckets[BucketCount];

	uint32_t shash(const string_t& str) const
	{
		uint32_t hash = 0;
		for (const char* c = str.c_str(); *c; ++c)
			hash = *c + (hash << 6) + (hash << 16) - hash;
		return hash;
	}
};

}

#endif	// traktor_StringMap_H
