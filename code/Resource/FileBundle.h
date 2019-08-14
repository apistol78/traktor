#pragma once

#include <map>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

/*! File bundle
 *  A file bundle is a serialized instance with a collection of named data streams attached.
 *
 * \ingroup Resource
 */
class T_DLLCLASS FileBundle : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Lookup name of data stream from identifier. */
	std::wstring lookup(const std::wstring& id) const;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class FileBundlePipeline;

	std::map< std::wstring, std::wstring > m_dataIds;
};

	}
}
