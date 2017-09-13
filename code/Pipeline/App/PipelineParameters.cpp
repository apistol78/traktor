/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Pipeline/App/PipelineParameters.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PipelineParameters", 0, PipelineParameters, ISerializable)

PipelineParameters::PipelineParameters()
:	m_verbose(false)
,	m_progress(false)
,	m_rebuild(false)
,	m_noCache(false)
{
}

PipelineParameters::PipelineParameters(
	const std::wstring& workingDirectory,
	const std::wstring& settings,
	bool verbose,
	bool progress,
	bool rebuild,
	bool noCache,
	const std::vector< Guid >& roots
)
:	m_workingDirectory(workingDirectory)
,	m_settings(settings)
,	m_verbose(verbose)
,	m_progress(progress)
,	m_rebuild(rebuild)
,	m_noCache(noCache)
,	m_roots(roots)
{
}

void PipelineParameters::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"workingDirectory", m_workingDirectory);
	s >> Member< std::wstring >(L"settings", m_settings);
	s >> Member< bool >(L"verbose", m_verbose);
	s >> Member< bool >(L"progress", m_progress);
	s >> Member< bool >(L"rebuild", m_rebuild);
	s >> Member< bool >(L"noCache", m_noCache);
	s >> MemberStlVector< Guid >(L"roots", m_roots);
}

}
