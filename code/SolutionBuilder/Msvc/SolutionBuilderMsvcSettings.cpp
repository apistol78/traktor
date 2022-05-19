#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcSettings.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXProj.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcSettings", 0, SolutionBuilderMsvcSettings, ISerializable)

void SolutionBuilderMsvcSettings::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"slnVersion", m_slnVersion);
	s >> Member< std::wstring >(L"vsVersion", m_vsVersion);
	s >> MemberRef< SolutionBuilderMsvcVCXProj >(L"project", m_project);
}

	}
}
