#include "Spark/Action/ActionStrings.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

static const std::string c_empty("");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ActionStrings", ActionStrings, Object)

uint32_t ActionStrings::operator [] (const std::string& str)
{
	auto it = m_strings.find(str);
	if (it != m_strings.end())
		return it->second;

	uint32_t id = m_next++;
	m_strings.insert(std::make_pair(str, id));

	return id;
}

const std::string& ActionStrings::operator [] (uint32_t id) const
{
	for (auto it = m_strings.begin(); it != m_strings.end(); ++it)
	{
		if (id == it->second)
			return it->first;
	}
	return c_empty;
}

	}
}
