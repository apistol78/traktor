#include "Amalgam/Editor/Deploy/Target.h"
#include "Amalgam/Editor/Deploy/TargetConfiguration.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.amalgam.Target", 0, Target, ISerializable)

void Target::setIdentifier(const std::wstring& identifier)
{
	m_identifier = identifier;
}

const std::wstring& Target::getIdentifier() const
{
	return m_identifier;
}

void Target::addConfiguration(TargetConfiguration* configuration)
{
	m_configurations.push_back(configuration);
}

void Target::removeConfiguration(TargetConfiguration* configuration)
{
	m_configurations.remove(configuration);
}

void Target::removeAllConfigurations()
{
	m_configurations.resize(0);
}

const RefArray< TargetConfiguration >& Target::getConfigurations() const
{
	return m_configurations;
}

void Target::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"identifier", m_identifier);
	s >> MemberRefArray< TargetConfiguration>(L"configurations", m_configurations);
}

	}
}
