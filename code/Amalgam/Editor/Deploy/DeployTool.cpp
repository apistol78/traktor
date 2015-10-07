#include "Amalgam/Editor/Deploy/DeployTool.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.DeployTool", 0, DeployTool, ISerializable)

const std::wstring& DeployTool::getExecutable() const
{
	return m_executable;
}

const std::map< std::wstring, std::wstring >& DeployTool::getEnvironment() const
{
	return m_environment;
}

void DeployTool::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"executable", m_executable);
	s >> MemberStlMap< std::wstring, std::wstring >(L"environment", m_environment);
}

	}
}
