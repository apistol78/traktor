#include "Editor/TypeBrowseFilter.h"
#include "Database/Instance.h"

namespace traktor
{
	namespace editor
	{

TypeBrowseFilter::TypeBrowseFilter(const Type& type)
:	m_type(type)
{
}

bool TypeBrowseFilter::acceptable(db::Instance* instance) const
{
	T_ASSERT (instance);
	T_ASSERT (instance->getPrimaryType());
	return is_type_of(m_type, *instance->getPrimaryType());
}

	}
}
