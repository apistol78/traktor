#include "Spark/Action/ActionDictionary.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ActionDictionary", ActionDictionary, Object)

ActionDictionary::ActionDictionary()
{
}

Ref< ActionDictionary > ActionDictionary::clone() const
{
	return new ActionDictionary(m_table);
}

void ActionDictionary::add(const ActionValue& value)
{
	m_table.push_back(value);
}

ActionDictionary::ActionDictionary(const AlignedVector< ActionValue >& table)
:	m_table(table)
{
}

	}
}
