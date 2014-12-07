#ifndef traktor_db_Traverse_H
#define traktor_db_Traverse_H

#include "Core/RefArray.h"

namespace traktor
{
	namespace db
	{

/*! \ingroup Database */
//@{

/*! \brief Find child group. */
template < typename GroupPredicate >
Ref< Group > findChildGroup(Group* group, const GroupPredicate& pred)
{
	RefArray< Group > childGroups;
	group->getChildGroups(childGroups);

	for (RefArray< Group >::iterator i = childGroups.begin(); i != childGroups.end(); ++i)
	{
		if (pred(*i))
			return *i;
	}

	return 0;
}

/*! \brief Find child instance. */
template < typename InstancePredicate >
Ref< Instance > findChildInstance(Group* group, const InstancePredicate& pred)
{
	RefArray< Instance > childInstances;
	group->getChildInstances(childInstances);

	for (RefArray< Instance >::iterator i = childInstances.begin(); i != childInstances.end(); ++i)
	{
		if (pred(*i))
			return *i;
	}

	return 0;
}

/*! \brief Find multiple child instances. */
template < typename InstancePredicate >
void findChildInstances(Group* group, const InstancePredicate& pred, RefArray< Instance >& outInstances)
{
	RefArray< Instance > childInstances;
	group->getChildInstances(childInstances);

	for (RefArray< Instance >::iterator i = childInstances.begin(); i != childInstances.end(); ++i)
	{
		if (pred(*i))
			outInstances.push_back(*i);
	}
}

/*! \brief Recursively find child group. */
template < typename GroupPredicate >
Ref< Group > recursiveFindChildGroup(Group* group, const GroupPredicate& pred)
{
	RefArray< Group > childGroups;
	group->getChildGroups(childGroups);

	for (RefArray< Group >::iterator i = childGroups.begin(); i != childGroups.end(); ++i)
	{
		if (pred(*i))
			return *i;
	}

	for (RefArray< Group >::iterator i = childGroups.begin(); i != childGroups.end(); ++i)
	{
		Ref< Group > childGroup = recursiveFindChildGroup(*i, pred);
		if (childGroup)
			return childGroup;
	}

	return 0;
}

/*! \brief Recursively find child instance. */
template < typename InstancePredicate >
Ref< Instance > recursiveFindChildInstance(Group* group, const InstancePredicate& pred)
{
	RefArray< Instance > childInstances;
	group->getChildInstances(childInstances);

	for (RefArray< Instance >::iterator i = childInstances.begin(); i != childInstances.end(); ++i)
	{
		if (pred(*i))
			return *i;
	}

	RefArray< Group > childGroups;
	group->getChildGroups(childGroups);

	for (RefArray< Group >::iterator i = childGroups.begin(); i != childGroups.end(); ++i)
	{
		Ref< Instance > instance = recursiveFindChildInstance(*i, pred);
		if (instance)
			return instance;
	}

	return 0;
}

/*! \brief Recursively find multiple child instances. */
template < typename InstancePredicate >
void recursiveFindChildInstances(Group* group, const InstancePredicate& pred, RefArray< Instance >& outInstances)
{
	RefArray< Instance > childInstances;
	group->getChildInstances(childInstances);

	for (RefArray< Instance >::iterator i = childInstances.begin(); i != childInstances.end(); ++i)
	{
		if (pred(*i))
			outInstances.push_back(*i);
	}

	RefArray< Group > childGroups;
	group->getChildGroups(childGroups);

	for (RefArray< Group >::iterator i = childGroups.begin(); i != childGroups.end(); ++i)
		recursiveFindChildInstances(*i, pred, outInstances);
}

/*! \brief Find group by name predicate. */
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

/*! \brief Find all instances predicate. */
struct FindInstanceAll
{
	bool operator () (const Instance* instance) const
	{
		return true;
	}
};

/*! \brief Find instance by guid predicate. */
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

/*! \brief Find instance by primary type predicate. */
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

/*! \brief Find instance by name predicate. */
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
}

#endif	// traktor_db_Traverse_H
