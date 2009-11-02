#include "Editor/PipelineHash.h"
#include "Core/Thread/Acquire.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.PipelineHash", PipelineHash, Serializable)

void PipelineHash::set(const Guid& guid, const Hash& hash)
{
	Acquire< Semaphore > lock(m_lock);
	m_hash[guid] = hash;
}

bool PipelineHash::get(const Guid& guid, Hash& outHash) const
{
	Acquire< Semaphore > lock(m_lock);

	std::map< Guid, Hash >::const_iterator i = m_hash.find(guid);
	if (i == m_hash.end())
		return false;

	outHash = i->second;
	return true;
}

bool PipelineHash::serialize(Serializer& s)
{
	return s >> MemberStlMap< Guid, Hash, MemberStlPair< Guid, Hash, Member< Guid >, MemberComposite< Hash > > >(L"hash", m_hash);
}

PipelineHash::Hash::Hash()
:	pipelineVersion(0)
,	pipelineHash(0)
,	sourceAssetHash(0)
{
}

bool PipelineHash::Hash::serialize(Serializer& s)
{
	s >> Member< uint32_t >(L"pipelineVersion", pipelineVersion);
	s >> Member< uint32_t >(L"pipelineHash", pipelineHash);
	s >> Member< uint32_t >(L"sourceAssetHash", sourceAssetHash);
	s >> MemberStlMap<
		Path,
		DateTime,
		MemberStlPair<
			Path,
			DateTime,
			Member< Path >,
			MemberComposite< DateTime >
		>
	>(L"timeStamps", timeStamps);
	return true;
}

	}
}
