#include "Amalgam/Editor/Feature.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.amalgam.Feature", 0, Feature, ISerializable)

void Feature::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"description", m_description);
	s >> MemberComposite< Platforms >(L"platforms", m_platforms);
	s >> MemberRef< PropertyGroup >(L"pipelineProperties", m_pipelineProperties);
	s >> MemberRef< PropertyGroup >(L"migrateProperties", m_migrateProperties);
	s >> MemberRef< PropertyGroup >(L"runtimeProperties", m_runtimeProperties);
}

Feature::Platforms::Platforms()
:	ios(false)
,	linuks(false)
,	mobile6(false)
,	osx(false)
,	ps3(false)
,	win32(false)
,	win64(false)
,	xbox360(false)
{
}

void Feature::Platforms::serialize(ISerializer& s)
{
	s >> Member< bool >(L"ios", ios);
	s >> Member< bool >(L"linux", linuks);
	s >> Member< bool >(L"mobile6", mobile6);
	s >> Member< bool >(L"osx", osx);
	s >> Member< bool >(L"ps3", ps3);
	s >> Member< bool >(L"win32", win32);
	s >> Member< bool >(L"win64", win64);
	s >> Member< bool >(L"xbox360", xbox360);
}

	}
}
