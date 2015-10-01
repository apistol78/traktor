#include "Amalgam/Editor/Feature.h"
#include "Amalgam/Editor/Platform.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.amalgam.Feature", 6, Feature, ISerializable)

Feature::Feature()
:	m_priority(0)
{
}

const Feature::Platform* Feature::getPlatform(const Guid& id) const
{
	for (std::list< Platform >::const_iterator i = m_platforms.begin(); i != m_platforms.end(); ++i)
	{
		if (i->platform == id)
			return &(*i);
	}
	return 0;
}

void Feature::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 4);

	s >> Member< std::wstring >(L"description", m_description);
	s >> Member< int32_t >(L"priority", m_priority);
	s >> MemberStlList< Platform, MemberComposite< Platform > >(L"platforms", m_platforms);
	s >> MemberRef< PropertyGroup >(L"pipelineProperties", m_pipelineProperties);
	s >> MemberRef< PropertyGroup >(L"migrateProperties", m_migrateProperties);
	s >> MemberRef< PropertyGroup >(L"runtimeProperties", m_runtimeProperties);

	if (s.getVersion() >= 6)
		s >> MemberStlMap< std::wstring, std::wstring >(L"environment", m_environment);
}

void Feature::Platform::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"platform", platform, AttributeType(type_of< traktor::amalgam::Platform >()));

	if (s.getVersion() >= 5)
		s >> Member< std::wstring >(L"executableFile", executableFile);

	s >> MemberStlList< std::wstring >(L"deployFiles", deployFiles);
}

	}
}
