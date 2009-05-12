#ifndef traktor_db_Traverse_H
#define traktor_db_Traverse_H

namespace traktor
{
	namespace db
	{

/*! \ingroup Database */
//@{

template < typename GroupPredicate >
Group* findChildGroup(Group* group, const GroupPredicate& pred)
{
	for (Ref< Group > childGroup = group->getFirstChildGroup(); childGroup; childGroup = group->getNextChildGroup(childGroup))
	{
		if (pred(childGroup))
			return childGroup;
	}
	return 0;
}

template < typename InstancePredicate >
Instance* findChildInstance(Group* group, const InstancePredicate& pred)
{
	for (Ref< Instance > childInstance = group->getFirstChildInstance(); childInstance; childInstance = group->getNextChildInstance(childInstance))
	{
		if (pred(childInstance))
			return childInstance;
	}
	return 0;
}

template < typename InstancePredicate >
void findChildInstances(Group* group, const InstancePredicate& pred, RefArray< Instance >& outInstances)
{
	for (Ref< Instance > childInstance = group->getFirstChildInstance(); childInstance; childInstance = group->getNextChildInstance(childInstance))
	{
		if (pred(childInstance))
			outInstances.push_back(childInstance);
	}
}

template < typename GroupPredicate >
Group* recursiveFindChildGroup(Group* group, const GroupPredicate& pred)
{
	Ref< Group > childGroup = findChildGroup(group, pred);
	if (childGroup)
		return childGroup;

	for (Ref< Group > childGroup = group->getFirstChildGroup(); childGroup; childGroup = group->getNextChildGroup(childGroup))
	{
		Ref< Group > childGroup = recursiveFindChildGroup(childGroup, pred);
		if (childGroup)
			return childGroup;
	}

	return 0;
}

template < typename InstancePredicate >
Instance* recursiveFindChildInstance(Group* group, const InstancePredicate& pred)
{
	Ref< Instance > childInstance = findChildInstance(group, pred);
	if (childInstance)
		return childInstance;

	for (Ref< Group > childGroup = group->getFirstChildGroup(); childGroup; childGroup = group->getNextChildGroup(childGroup))
	{
		Ref< Instance > instance = recursiveFindChildInstance(childGroup, pred);
		if (instance)
			return instance;
	}

	return 0;
}

template < typename InstancePredicate >
void recursiveFindChildInstances(Group* group, const InstancePredicate& pred, RefArray< Instance >& outInstances)
{
	findChildInstances(group, pred, outInstances);
	for (Ref< Group > childGroup = group->getFirstChildGroup(); childGroup; childGroup = group->getNextChildGroup(childGroup))
		recursiveFindChildInstances(childGroup, pred, outInstances);
}

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

struct FindInstanceByType
{
	const Type& instanceType;

	FindInstanceByType(const Type& instanceType_)
	:	instanceType(instanceType_)
	{
	}

	bool operator () (const Instance* instance) const
	{
		if (!instance->getPrimaryType())
			return false;
		return is_type_of(instanceType, *instance->getPrimaryType());
	}
};

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
