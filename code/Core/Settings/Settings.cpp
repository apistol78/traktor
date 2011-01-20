#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/Settings.h"

namespace traktor
{
	namespace
	{

Ref< const IPropertyValue > traverseGetProperty(Ref< const PropertyGroup > group, const std::wstring& propertyName)
{
	size_t fpos = 0;
	for (;;)
	{
		size_t epos = propertyName.find(L'/', fpos);
		if (epos == propertyName.npos)
		{
			return group->getProperty(propertyName.substr(fpos));
		}
		else
		{
			group = group->getProperty< PropertyGroup >(propertyName.substr(fpos, epos - fpos));
			if (!group)
				return 0;

			fpos = epos + 1;
		}
	}
}

void traverseSetProperty(Ref< PropertyGroup > group, const std::wstring& propertyName, IPropertyValue* value)
{
	size_t epos = propertyName.find(L'/');
	if (epos == propertyName.npos)
	{
		group->setProperty(propertyName, value);
	}
	else
	{
		Ref< PropertyGroup > childGroup = group->getProperty< PropertyGroup >(propertyName.substr(0, epos));
		if (!childGroup)
		{
			if (value)
				childGroup = new PropertyGroup();
			else
				return;
		}
		traverseSetProperty(childGroup, propertyName.substr(epos + 1), value);
		group->setProperty(propertyName.substr(0, epos), childGroup);
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Settings", Settings, Object)

Settings::Settings()
:	m_rootGroup(new PropertyGroup())
{
}

Settings::Settings(PropertyGroup* rootGroup)
:	m_rootGroup(rootGroup)
{
}

Ref< Settings > Settings::read(ISerializer& s)
{
	T_ASSERT (s.getDirection() == ISerializer::SdRead);

	Ref< PropertyGroup > rootGroup;
	if (!(s >> MemberRef< PropertyGroup >(L"object", rootGroup)))
		return false;

	return new Settings(rootGroup);
}

bool Settings::write(ISerializer& s) const
{
	T_ASSERT (s.getDirection() == ISerializer::SdWrite);
	ISerializable* mutableRootGroup = (ISerializable*)(m_rootGroup);
	return s >> Member< ISerializable* >(L"object", mutableRootGroup);
}

Ref< Settings > Settings::clone() const
{
	Ref< PropertyGroup > clonedRootGroup = DeepClone(m_rootGroup).create< PropertyGroup >();
	Ref< Settings > clonedSettings = new Settings();
	clonedSettings->m_rootGroup = clonedRootGroup;
	return clonedSettings;
}

void Settings::setProperty(const std::wstring& propertyName, IPropertyValue* value)
{
	traverseSetProperty(m_rootGroup, propertyName, value);
}

Ref< const IPropertyValue > Settings::getProperty(const std::wstring& propertyName) const
{
	return traverseGetProperty(m_rootGroup, propertyName);
}

void Settings::merge(Settings* right, bool join)
{
	const std::map< std::wstring, Ref< IPropertyValue > >& rightValues = right->m_rootGroup->getValues();
	for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = rightValues.begin(); i != rightValues.end(); ++i)
	{
		Ref< IPropertyValue > leftValue = m_rootGroup->getProperty(i->first);
		if (leftValue)
			m_rootGroup->setProperty(i->first, leftValue->merge(i->second, join));
		else
			m_rootGroup->setProperty(i->first, i->second);
	}
}

}
