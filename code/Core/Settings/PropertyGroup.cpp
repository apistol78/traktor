#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Settings/PropertyGroup.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLONABLE_CLASS(L"traktor.PropertyGroup", 0, PropertyGroup, IPropertyValue)

PropertyGroup::PropertyGroup()
{
}

PropertyGroup::value_type_t PropertyGroup::get(const IPropertyValue* value)
{
	return value ? DeepClone(value).create< PropertyGroup >() : 0;
}

void PropertyGroup::setProperty(const std::wstring& propertyName, IPropertyValue* value)
{
	size_t pos = propertyName.find(L'/');
	if (pos == propertyName.npos)
	{
		if (value)
			m_value[propertyName] = value;
		else
			m_value.erase(propertyName);
	}
	else
	{
		std::wstring groupName = propertyName.substr(0, pos);
		Ref< PropertyGroup > group;

		std::map< std::wstring, Ref< IPropertyValue > >::const_iterator it = m_value.find(groupName);
		if (it != m_value.end())
		{
			group = dynamic_type_cast< PropertyGroup* >(it->second);
			if (!group)
				return;
		}
		else
		{
			group = new PropertyGroup();
			m_value[groupName] = group;
		}

		group->setProperty(propertyName.substr(pos + 1), value);
	}
}

IPropertyValue* PropertyGroup::getProperty(const std::wstring& propertyName)
{
	size_t pos = propertyName.find(L'/');
	if (pos == propertyName.npos)
	{
		std::map< std::wstring, Ref< IPropertyValue > >::const_iterator it = m_value.find(propertyName);
		return it != m_value.end() ? it->second.ptr() : 0;
	}
	else
	{
		std::wstring groupName = propertyName.substr(0, pos);

		std::map< std::wstring, Ref< IPropertyValue > >::const_iterator it = m_value.find(groupName);
		if (it == m_value.end())
			return 0;

		PropertyGroup* group = dynamic_type_cast< PropertyGroup* >(it->second);
		if (!group)
			return 0;

		return group->getProperty(propertyName.substr(pos + 1));
	}
}

const IPropertyValue* PropertyGroup::getProperty(const std::wstring& propertyName) const
{
	size_t pos = propertyName.find(L'/');
	if (pos == propertyName.npos)
	{
		std::map< std::wstring, Ref< IPropertyValue > >::const_iterator it = m_value.find(propertyName);
		return it != m_value.end() ? it->second.ptr() : 0;
	}
	else
	{
		std::wstring groupName = propertyName.substr(0, pos);

		std::map< std::wstring, Ref< IPropertyValue > >::const_iterator it = m_value.find(groupName);
		if (it == m_value.end())
			return 0;

		PropertyGroup* group = dynamic_type_cast< PropertyGroup* >(it->second);
		if (!group)
			return 0;

		return group->getProperty(propertyName.substr(pos + 1));
	}
}

Ref< PropertyGroup > PropertyGroup::mergeJoin(const PropertyGroup* rightGroup) const
{
	const std::map< std::wstring, Ref< IPropertyValue > >& leftValues = getValues();
	const std::map< std::wstring, Ref< IPropertyValue > >& rightValues = rightGroup->getValues();

	Ref< PropertyGroup > joinedGroup = new PropertyGroup();

	// Insert values from left group.
	for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = leftValues.begin(); i != leftValues.end(); ++i)
	{
		if (rightValues.find(i->first) != rightValues.end())
			continue;

		joinedGroup->setProperty(i->first, DeepClone(i->second).create< IPropertyValue >());
	}

	// Insert values from right group.
	for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = rightValues.begin(); i != rightValues.end(); ++i)
	{
		std::map< std::wstring, Ref< IPropertyValue > >::const_iterator it = leftValues.find(i->first);
		if (it != leftValues.end())
			joinedGroup->setProperty(i->first, it->second->join(i->second));
		else
			joinedGroup->setProperty(i->first, DeepClone(i->second).create< IPropertyValue >());
	}

	return joinedGroup;
}

Ref< PropertyGroup > PropertyGroup::mergeReplace(const PropertyGroup* rightGroup) const
{
	const std::map< std::wstring, Ref< IPropertyValue > >& leftValues = getValues();
	const std::map< std::wstring, Ref< IPropertyValue > >& rightValues = rightGroup->getValues();

	Ref< PropertyGroup > joinedGroup = new PropertyGroup();

	// Insert values from left group.
	for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = leftValues.begin(); i != leftValues.end(); ++i)
	{
		if (rightValues.find(i->first) != rightValues.end())
			continue;

		joinedGroup->setProperty(i->first, DeepClone(i->second).create< IPropertyValue >());
	}

	// Insert values from right group.
	for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = rightValues.begin(); i != rightValues.end(); ++i)
		joinedGroup->setProperty(i->first, DeepClone(i->second).create< IPropertyValue >());

	return joinedGroup;
}

bool PropertyGroup::serialize(ISerializer& s)
{
	return s >> MemberStlMap<
		std::wstring,
		Ref< IPropertyValue >,
		MemberStlPair<
			std::wstring,
			Ref< IPropertyValue >,
			Member< std::wstring >,
			MemberRef< IPropertyValue >
		>
	>(L"value", m_value);
}

Ref< IPropertyValue > PropertyGroup::join(const IPropertyValue* right) const
{
	if (const PropertyGroup* rightGroup = dynamic_type_cast< const PropertyGroup* >(right))
		return mergeJoin(rightGroup);
	else
		return clone_instance(right);
}

}
