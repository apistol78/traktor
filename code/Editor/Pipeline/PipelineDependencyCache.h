#ifndef traktor_editor_PipelineDependencyCache_H
#define traktor_editor_PipelineDependencyCache_H

#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Semaphore.h"

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

class T_DLLCLASS PipelineDependencyCache : public ISerializable
{
	T_RTTI_CLASS;

public:
	void put(const Guid& dependencyGuid, PipelineDependency* dependency);

	PipelineDependency* get(const Guid& dependencyGuid) const;

	virtual bool serialize(ISerializer& s);

private:
	mutable Semaphore m_lock;
	std::map< Guid, Ref< PipelineDependency > > m_dependencies;
};

	}
}

#endif	// traktor_editor_PipelineDependencyCache_H
