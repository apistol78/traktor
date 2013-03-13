#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Editor/Pipeline/AgentStream.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.AgentStream", 0, AgentStream, ISerializable)

AgentStream::AgentStream()
{
}

AgentStream::AgentStream(uint32_t publicId)
:	m_publicId(publicId)
{
}

uint32_t AgentStream::getPublicId() const
{
	return m_publicId;
}

bool AgentStream::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"publicId", m_publicId);
	return true;
}

	}
}
