#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"

namespace traktor
{
	namespace spark
	{

class ActionStrings : public Object
{
	T_RTTI_CLASS;

public:
	uint32_t operator [] (const std::string& str);

	const std::string& operator [] (uint32_t id) const;

private:
	SmallMap< std::string, uint32_t > m_strings;
	uint32_t m_next = 0;
};

	}
}

