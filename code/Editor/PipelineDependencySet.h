#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class OutputStream;

}

namespace traktor::editor
{

class PipelineDependency;

class T_DLLCLASS PipelineDependencySet : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum
	{
		DiInvalid = ~0U
	};

	uint32_t add(PipelineDependency* dependency);

	uint32_t add(const Guid& dependencyGuid, PipelineDependency* dependency);

	PipelineDependency* get(uint32_t index);

	const PipelineDependency* get(uint32_t index) const;

	uint32_t get(const Guid& dependencyGuid) const;

	uint32_t size() const;

	void dump(OutputStream& os) const;

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< PipelineDependency > m_dependencies;
	SmallMap< Guid, uint32_t > m_indices;
};

}
