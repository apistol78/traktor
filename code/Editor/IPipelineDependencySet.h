#ifndef traktor_editor_IPipelineDependencySet_H
#define traktor_editor_IPipelineDependencySet_H

#include "Core/Guid.h"
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
	namespace editor
	{

class PipelineDependency;

class T_DLLCLASS IPipelineDependencySet : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum
	{
		DiInvalid = ~0U
	};

	virtual uint32_t add(PipelineDependency* dependency) = 0;

	virtual uint32_t add(const Guid& dependencyGuid, PipelineDependency* dependency) = 0;

	virtual PipelineDependency* get(uint32_t index) const = 0;

	virtual uint32_t get(const Guid& dependencyGuid) const = 0;

	virtual uint32_t size() const = 0;

	virtual void dump() = 0;
};

	}
}

#endif	// traktor_editor_IPipelineDependencySet_H
