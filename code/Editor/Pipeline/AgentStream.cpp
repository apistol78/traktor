/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Editor/Pipeline/AgentStream.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.AgentStream", 0, AgentStream, ISerializable)

AgentStream::AgentStream()
:	m_publicId(0)
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

void AgentStream::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"publicId", m_publicId);
}

	}
}
