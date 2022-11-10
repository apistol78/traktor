/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Core/Settings/PropertyGroup.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PropertyGroup", 0, PropertyGroup, IPropertyValue)

PropertyGroup::PropertyGroup()
{
}

PropertyGroup::PropertyGroup(const SmallMap< std::wstring, Ref< IPropertyValue > >& value)
:	m_value(value)
{
}

PropertyGroup::value_type_t PropertyGroup::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< PropertyGroup* >(value->clone()) : nullptr;
}

void PropertyGroup::setProperty(const std::wstring& propertyName, IPropertyValue* value)
{
	size_t pos = propertyName.find(L'/');
	if (pos == propertyName.npos)
	{
		if (value)
			m_value[propertyName] = value;
		else
			m_value.remove(propertyName);
	}
	else
	{
		std::wstring groupName = propertyName.substr(0, pos);
		Ref< PropertyGroup > group;

		auto it = m_value.find(groupName);
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
		auto it = m_value.find(propertyName);
		return it != m_value.end() ? it->second.ptr() : nullptr;
	}
	else
	{
		std::wstring groupName = propertyName.substr(0, pos);

		auto it = m_value.find(groupName);
		if (it == m_value.end())
			return nullptr;

		PropertyGroup* group = dynamic_type_cast< PropertyGroup* >(it->second);
		if (!group)
			return nullptr;

		return group->getProperty(propertyName.substr(pos + 1));
	}
}

const IPropertyValue* PropertyGroup::getProperty(const std::wstring& propertyName) const
{
	size_t pos = propertyName.find(L'/');
	if (pos == propertyName.npos)
	{
		auto it = m_value.find(propertyName);
		return it != m_value.end() ? it->second.ptr() : nullptr;
	}
	else
	{
		std::wstring groupName = propertyName.substr(0, pos);

		auto it = m_value.find(groupName);
		if (it == m_value.end())
			return nullptr;

		PropertyGroup* group = dynamic_type_cast< PropertyGroup* >(it->second);
		if (!group)
			return nullptr;

		return group->getProperty(propertyName.substr(pos + 1));
	}
}

Ref< PropertyGroup > PropertyGroup::merge(const PropertyGroup* rightGroup, MergeMode mode) const
{
	const auto& leftValues = getValues();
	const auto& rightValues = rightGroup->getValues();

	Ref< PropertyGroup > mergedGroup = new PropertyGroup();

	// Insert values from left group.
	for (auto i = leftValues.begin(); i != leftValues.end(); ++i)
	{
		if (!i->second || rightValues.find(i->first) != rightValues.end())
			continue;

		mergedGroup->setProperty(i->first, i->second->clone());
	}

	// Insert values from right group.
	for (auto i = rightValues.begin(); i != rightValues.end(); ++i)
	{
		auto it = leftValues.find(i->first);
		if (mode == MmJoin && it != leftValues.end() && it->second)
			mergedGroup->setProperty(i->first, it->second->join(i->second));
		else if (i->second)
			mergedGroup->setProperty(i->first, i->second->clone());
	}

	return mergedGroup;
}

Ref< PropertyGroup > PropertyGroup::difference(const PropertyGroup* rightGroup) const
{
	const auto& leftValues = getValues();
	const auto& rightValues = rightGroup->getValues();

	Ref< PropertyGroup > diffGroup = new PropertyGroup();

	for (auto il = leftValues.begin(); il != leftValues.end(); ++il)
	{
		auto ir = rightValues.find(il->first);
		if (ir != rightValues.end())
		{
			// Ensure both left and right values aren't nil.
			if (il->second && ir->second)
			{
				if (&type_of(il->second) == &type_of(ir->second))
				{
					if (is_a< PropertyGroup >(il->second))
					{
						Ref< PropertyGroup > d = static_cast< const PropertyGroup* >(il->second.c_ptr())->difference(static_cast< const PropertyGroup* >(ir->second.c_ptr()));
						if (d && !d->getValues().empty())
							diffGroup->setProperty(il->first, d);
					}
					else
					{
						// Types match and are not groups, check if value match and if not then keep right value.
						if (DeepHash(il->second) != DeepHash(ir->second))
							diffGroup->setProperty(il->first, ir->second->clone());
					}
				}
				else
				{
					// Types mismatch, keep right value.
					diffGroup->setProperty(il->first, ir->second->clone());
				}
			}
			else if (ir->second)
			{
				// Left value are nil, keep right value.
				diffGroup->setProperty(il->first, ir->second->clone());
			}
		}
	}

	// Add all right values which isn't present in left group.
	for (auto ir = rightValues.begin(); ir != rightValues.end(); ++ir)
	{
		if (!ir->second)
			continue;

		auto il = leftValues.find(ir->first);
		if (il == leftValues.end())
			diffGroup->setProperty(ir->first, ir->second->clone());
	}

	return diffGroup;
}

void PropertyGroup::serialize(ISerializer& s)
{
	s >> MemberSmallMap<
		std::wstring,
		Ref< IPropertyValue >,
		Member< std::wstring >,
		MemberRef< IPropertyValue >
	>(L"value", m_value);
}

Ref< IPropertyValue > PropertyGroup::join(const IPropertyValue* right) const
{
	if (const PropertyGroup* rightGroup = dynamic_type_cast< const PropertyGroup* >(right))
		return merge(rightGroup, MmJoin);
	else
		return right->clone();
}

Ref< IPropertyValue > PropertyGroup::clone() const
{
	SmallMap< std::wstring, Ref< IPropertyValue > > value;
	for (auto i = m_value.begin(); i != m_value.end(); ++i)
	{
		if (!i->second)
			continue;
		value.insert(std::make_pair(
			i->first,
			i->second->clone()
		));
	}
	return new PropertyGroup(value);
}

}
