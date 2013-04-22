#ifndef traktor_resource_ResourceBundleAsset_H
#define traktor_resource_ResourceBundleAsset_H

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class T_DLLCLASS ResourceBundleAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	ResourceBundleAsset();

	const std::vector< Guid >& get() const;

	bool persistent() const;

	virtual bool serialize(ISerializer& s);

private:
	std::vector< Guid > m_resources;
	bool m_persistent;
};

	}
}

#endif	// traktor_resource_ResourceBundleAsset_H
