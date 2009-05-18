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

int PipelineHash::getVersion() const
{
	return 1;
}

bool PipelineHash::serialize(Serializer& s)
{
	if (s.getVersion() >= 1)
		return s >> MemberStlMap< Guid, Hash, MemberStlPair< Guid, Hash, Member< Guid >, MemberComposite< Hash > > >(L"hash", m_hash);
	else
		return true;
}

PipelineHash::Hash::Hash()
:	pipelineVersion(0)
{
}

bool PipelineHash::Hash::serialize(Serializer& s)
{
	s >> MemberComposite< MD5 >(L"checksum", checksum);
	s >> MemberComposite< DateTime >(L"assetTimestamp", assetTimestamp);
	s >> Member< uint32_t >(L"pipelineVersion", pipelineVersion);
	return true;
}

	}
}
