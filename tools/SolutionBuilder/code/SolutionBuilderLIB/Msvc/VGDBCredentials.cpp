#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include "SolutionBuilderLIB/Msvc/VGDBCredentials.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"VGDBCredentials", 0, VGDBCredentials, ISerializable)

void VGDBCredentials::serialize(traktor::ISerializer& s)
{
	s >> Member< std::wstring >(L"host", m_host);
	s >> Member< std::wstring >(L"user", m_user);
	s >> Member< std::wstring >(L"localPath", m_localPath);
	s >> Member< std::wstring >(L"remotePath", m_remotePath);
}