#include <cstring>
#include "Flash/Action/ActionDictionary.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionDictionary", ActionDictionary, Object)

ActionDictionary::ActionDictionary(uint16_t tableSize, const char* tableFirstEntry)
{
	m_table.resize(tableSize);
	for (int i = 0; i < tableSize; ++i)
	{
		m_table[i] = tableFirstEntry;
		tableFirstEntry += strlen(tableFirstEntry) + 1;
	}
}

	}
}
