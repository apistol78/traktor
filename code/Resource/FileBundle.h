#ifndef traktor_resource_FileBundle_H
#define traktor_resource_FileBundle_H

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
	
class T_DLLCLASS FileBundle : public ISerializable
{
	T_RTTI_CLASS;

public:
	std::wstring lookup(const std::wstring& id) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class FileBundlePipeline;

	std::map< std::wstring, std::wstring > m_dataIds;
};
	
	}
}

#endif	// traktor_resource_FileBundle_H

