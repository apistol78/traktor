#pragma once

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

/*!
 * \ingroup Resource
 */
class T_DLLCLASS ResourceBundleAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	void add(const Guid& resource);

	const AlignedVector< Guid >& get() const;

	void setPersistent(bool persistent);

	bool persistent() const;

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< Guid > m_resources;
	bool m_persistent = false;
};

	}
}

