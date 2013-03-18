#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Thread/Acquire.h"
#include "Editor/Pipeline/PipelineDependency.h"
#include "Editor/Pipeline/PipelineDependencyCache.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PipelineDependencyCache", 0, PipelineDependencyCache, ISerializable)

void PipelineDependencyCache::put(const Guid& dependencyGuid, PipelineDependency* dependency)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_dependencies[dependencyGuid] = dependency;
}

PipelineDependency* PipelineDependencyCache::get(const Guid& dependencyGuid) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	std::map< Guid, Ref< PipelineDependency > >::const_iterator i = m_dependencies.find(dependencyGuid);
	if (i != m_dependencies.end())
		return i->second;
	else
		return 0;
}

bool PipelineDependencyCache::serialize(ISerializer& s)
{
	return s >> MemberStlMap< Guid, Ref< PipelineDependency  >, MemberStlPair< Guid, Ref< PipelineDependency >, Member< Guid >, MemberRef< PipelineDependency > > >(L"dependencies", m_dependencies);
}

	}
}
