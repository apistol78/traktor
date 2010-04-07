#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Online/Editor/AchievementDesc.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.AchievementDesc", 0, AchievementDesc, ISerializable)

AchievementDesc::AchievementDesc()
:	m_hidden(false)
{
}

void AchievementDesc::setId(const std::wstring& id)
{
	m_id = id;
}

const std::wstring& AchievementDesc::getId() const
{
	return m_id;
}

void AchievementDesc::setUnachievedImage(const Path& unachievedImage)
{
	m_unachievedImage = unachievedImage;
}

const Path& AchievementDesc::getUnachievedImage() const
{
	return m_unachievedImage;
}

void AchievementDesc::setAchievedImage(const Path& achievedImage)
{
	m_achievedImage = achievedImage;
}

const Path& AchievementDesc::getAchievedImage() const
{
	return m_achievedImage;
}

void AchievementDesc::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& AchievementDesc::getName() const
{
	return m_name;
}

void AchievementDesc::setDescription(const std::wstring& description)
{
	m_description = description;
}

const std::wstring& AchievementDesc::getDescription() const
{
	return m_description;
}

void AchievementDesc::setHidden(bool hidden)
{
	m_hidden = hidden;
}

bool AchievementDesc::getHidden() const
{
	return m_hidden;
}

bool AchievementDesc::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"id", m_id);
	s >> Member< Path >(L"unachievedImage", m_unachievedImage);
	s >> Member< Path >(L"achievedImage", m_achievedImage);
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"description", m_description);
	s >> Member< bool >(L"hidden", m_hidden);
	return true;
}

	}
}
