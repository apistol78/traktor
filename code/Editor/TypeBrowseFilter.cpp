#include "Editor/TypeBrowseFilter.h"
#include "Database/Instance.h"

namespace traktor
{
	namespace editor
	{

TypeBrowseFilter::TypeBrowseFilter(const TypeInfoSet& typeSet)
:	m_typeSet(typeSet)
{
}

TypeBrowseFilter::TypeBrowseFilter(const TypeInfo& typeInfo)
{
	m_typeSet.insert(&typeInfo);
}

bool TypeBrowseFilter::acceptable(db::Instance* instance) const
{
	T_ASSERT (instance);
	T_ASSERT (instance->getPrimaryType());

	for (TypeInfoSet::const_iterator i = m_typeSet.begin(); i != m_typeSet.end(); ++i)
	{
		if (is_type_of(*(*i), *instance->getPrimaryType()))
			return true;
	}

	return false;
}

	}
}
