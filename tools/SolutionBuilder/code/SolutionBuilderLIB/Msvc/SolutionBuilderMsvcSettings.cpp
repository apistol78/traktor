#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberRef.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcSettings.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcProject.h"

using namespace traktor;

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"SolutionBuilderMsvcSettings", SolutionBuilderMsvcSettings, Serializable)

bool SolutionBuilderMsvcSettings::serialize(Serializer& s)
{
	s >> Member< std::wstring >(L"slnVersion", m_slnVersion);
	s >> Member< std::wstring >(L"vsVersion", m_vsVersion);
	s >> MemberRef< SolutionBuilderMsvcProject >(L"project", m_project);
	return true;
}
