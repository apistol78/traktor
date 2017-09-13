/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Pipeline/App/PipelineResult.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"PipelineResult", 0, PipelineResult, ISerializable)

PipelineResult::PipelineResult()
:	m_result(0)
{
}

PipelineResult::PipelineResult(int32_t result)
:	m_result(result)
{
}

void PipelineResult::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"result", m_result);
}

}
