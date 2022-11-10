#pragma once

#include "Core/RefArray.h"

namespace traktor::db
{

/*! \ingroup Database */
//@{

/*! Find child group. */
template < typename GroupPredicate >
Ref< Group > findChildGroup(Group* group, const GroupPredicate& pred)
{
	RefArray< Group > childGroups;
	group->getChildGroups(childGroups);

	for (const auto childGroup : childGroups)
	{
		if (pred(childGroup))
			return childGroup;
	}

	return nullptr;
}

/*! Find child instance. */
template < typename InstancePredicate >
Ref< Instance > findChildInstance(Group* group, const InstancePredicate& pred)
{
	RefArray< Instance > childInstances;
	group->getChildInstances(childInstances);

	for (const auto childInstance : childInstances)
	{
		if (pred(childInstance))
			return childInstance;
	}

	return nullptr;
}

/*! Find multiple child instances. */
template < typename InstancePredicate >
void findChildInstances(Group* group, const InstancePredicate& pred, RefArray< Instance >& outInstances)
{
	RefArray< Instance > childInstances;
	group->getChildInstances(childInstances);

	for (const auto childInstance : childInstances)
	{
		if (pred(childInstance))
			outInstances.push_back(childInstance);
	}
}

/*! Recursively find child group. */
template < typename GroupPredicate >
Ref< Group > recursiveFindChildGroup(Group* group, const GroupPredicate& pred)
{
	RefArray< Group > childGroups;
	group->getChildGroups(childGroups);

	for (const auto childGroup : childGroups)
	{
		if (pred(childGroup))
			return childGroup;
	}

	for (const auto childGroup : childGroups)
	{
		Ref< Group > foundChildGroup = recursiveFindChildGroup(childGroup, pred);
		if (foundChildGroup)
			return foundChildGroup;
	}

	return nullptr;
}

/*! Recursively find child instance. */
template < typename InstancePredicate >
Ref< Instance > recursiveFindChildInstance(Group* group, const InstancePredicate& pred)
{
	RefArray< Instance > childInstances;
	group->getChildInstances(childInstances);

	for (const auto childInstance : childInstances)
	{
		if (pred(childInstance))
			return childInstance;
	}

	RefArray< Group > childGroups;
	group->getChildGroups(childGroups);

	for (const auto childGroup : childGroups)
	{
		Ref< Instance > instance = recursiveFindChildInstance(childGroup, pred);
		if (instance)
			return instance;
	}

	return nullptr;
}

/*! Recursively find multiple child instances. */
template < typename InstancePredicate >
void recursiveFindChildInstances(Group* group, const InstancePredicate& pred, RefArray< Instance >& outInstances)
{
	RefArray< Instance > childInstances;
	group->getChildInstances(childInstances);

	for (const auto childInstance : childInstances)
	{
		if (pred(childInstance))
			outInstances.push_back(childInstance);
	}

	RefArray< Group > childGroups;
	group->getChildGroups(childGroups);

	for (const auto childGroup : childGroups)
		recursiveFindChildInstances(childGroup, pred, outInstances);
}

/*! Find group by name predicate. */
struct FindGroupByName
{
	const std::wstring& groupName;

	FindGroupByName(const std::wstring& groupName_)
	:	groupName(groupName_)
	{
	}

	bool operator () (const Group* group) const
	{
		return group->getName() == groupName;
	}
};

/*! Find all instances predicate. */
struct FindInstanceAll
{
	bool operator () (const Instance* instance) const
	{
		return true;
	}
};

/*! Find instance by guid predicate. */
struct FindInstanceByGuid
{
	const Guid& instanceGuid;

	FindInstanceByGuid(const Guid& instanceGuid_)
	:	instanceGuid(instanceGuid_)
	{
	}

	bool operator () (const Instance* instance) const
	{
		return instance->getGuid() == instanceGuid;
	}
};

/*! Find instance by primary type predicate. */
struct FindInstanceByType
{
	const TypeInfo& instanceType;

	FindInstanceByType(const TypeInfo& instanceType_)
	:	instanceType(instanceType_)
	{
	}

	bool operator () (const Instance* instance) const
	{
		const TypeInfo* primaryType = instance->getPrimaryType();
		if (!primaryType)
			return false;
		return is_type_of(instanceType, *primaryType);
	}
};

/*! Find instance by name predicate. */
struct FindInstanceByName
{
	const std::wstring& instanceName;

	FindInstanceByName(const std::wstring& instanceName_)
	:	instanceName(instanceName_)
	{
	}

	bool operator () (const Instance* instance) const
	{
		return instance->getName() == instanceName;
	}
};

//@}

}
