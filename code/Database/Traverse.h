#ifndef traktor_db_Traverse_H
#define traktor_db_Traverse_H

namespace traktor
{
	namespace db
	{

/*! \ingroup Database */
//@{

template < typename GroupPredicate >
Ref< Group > findChildGroup(Group* group, const GroupPredicate& pred)
{
	for (RefArray< Group >::iterator i = group->getBeginChildGroup(); i != group->getEndChildGroup(); ++i)
	{
		if (pred(*i))
			return *i;
	}
	return 0;
}

template < typename InstancePredicate >
Ref< Instance > findChildInstance(Group* group, const InstancePredicate& pred)
{
	for (RefArray< Instance >::iterator i = group->getBeginChildInstance(); i != group->getEndChildInstance(); ++i)
	{
		if (pred(*i))
			return *i;
	}
	return 0;
}

template < typename InstancePredicate >
void findChildInstances(Group* group, const InstancePredicate& pred, RefArray< Instance >& outInstances)
{
	for (RefArray< Instance >::iterator i = group->getBeginChildInstance(); i != group->getEndChildInstance(); ++i)
	{
		if (pred(*i))
			outInstances.push_back(*i);
	}
}

template < typename GroupPredicate >
Ref< Group > recursiveFindChildGroup(Group* group, const GroupPredicate& pred)
{
	Ref< Group > childGroup = findChildGroup(group, pred);
	if (childGroup)
		return childGroup;

	for (RefArray< Group >::iterator i = group->getBeginChildGroup(); i != group->getEndChildGroup(); ++i)
	{
		Ref< Group > childGroup = recursiveFindChildGroup(*i, pred);
		if (childGroup)
			return childGroup;
	}

	return 0;
}

template < typename InstancePredicate >
Ref< Instance > recursiveFindChildInstance(Group* group, const InstancePredicate& pred)
{
	Ref< Instance > childInstance = findChildInstance(group, pred);
	if (childInstance)
		return childInstance;

	for (RefArray< Group >::iterator i = group->getBeginChildGroup(); i != group->getEndChildGroup(); ++i)
	{
		Ref< Instance > instance = recursiveFindChildInstance(*i, pred);
		if (instance)
			return instance;
	}

	return 0;
}

template < typename InstancePredicate >
void recursiveFindChildInstances(Group* group, const InstancePredicate& pred, RefArray< Instance >& outInstances)
{
	findChildInstances(group, pred, outInstances);
	for (RefArray< Group >::iterator i = group->getBeginChildGroup(); i != group->getEndChildGroup(); ++i)
		recursiveFindChildInstances(*i, pred, outInstances);
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
