/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Pipeline/App/PipelineLog.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PipelineLog", 0, PipelineLog, ISerializable)

PipelineLog::PipelineLog()
:	m_threadId(0)
,	m_level(0)
{
}

PipelineLog::PipelineLog(uint32_t threadId, int32_t level, const std::wstring& text)
:	m_threadId(threadId)
,	m_level(level)
,	m_text(text)
{
}

void PipelineLog::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"threadId", m_threadId);
	s >> Member< int32_t >(L"level", m_level);
	s >> Member< std::wstring >(L"text", m_text);
}

}
