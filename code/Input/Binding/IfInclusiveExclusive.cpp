#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStl.h"
#include "Input/Binding/IfInclusiveExclusive.h"
#include "Input/Binding/InputValueSet.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.IfInclusiveExclusive", 0, IfInclusiveExclusive, IInputFilter)

void IfInclusiveExclusive::evaluate(InputValueSet& valueSet) const
{
	if (m_priority == PrInclusive)
	{
		bool resetExclusive = false;
	
		// Check if any inclusive value is active.
		for (std::list< std::wstring >::const_iterator i = m_inclusive.begin(); i != m_inclusive.end(); ++i)
		{
			float value = valueSet.get(*i);
			if (asBoolean(value))
			{
				resetExclusive = true;
				break;
			}
		}
	
		// If no inclusive active, ensure only a single exclusive value is active.
		if (!resetExclusive)
		{
			uint32_t exclusiveCount = 0;
			for (std::list< std::wstring >::const_iterator i = m_exclusive.begin(); i != m_exclusive.end(); ++i)
			{
				float value = valueSet.get(*i);
				if (asBoolean(value))
					++exclusiveCount;
			}
		
			// Reset exclusive if more than one is active.
			if (exclusiveCount > 1)
				resetExclusive = true;
		}
	
		if (resetExclusive)
		{
			for (std::list< std::wstring >::const_iterator i = m_exclusive.begin(); i != m_exclusive.end(); ++i)
				valueSet.set(*i, asFloat(false));
		}
	}
	else if (m_priority == PrExclusive)
	{
		uint32_t exclusiveCount = 0;
		for (std::list< std::wstring >::const_iterator i = m_exclusive.begin(); i != m_exclusive.end(); ++i)
		{
			float value = valueSet.get(*i);
			if (asBoolean(value))
				++exclusiveCount;
		}
		
		if (exclusiveCount == 1)
		{
			// Reset inclusive values.
			for (std::list< std::wstring >::const_iterator i = m_inclusive.begin(); i != m_inclusive.end(); ++i)
				valueSet.set(*i, asFloat(false));
		}
		else if (exclusiveCount > 1)
		{
			// Reset exclusive values.
			for (std::list< std::wstring >::const_iterator i = m_exclusive.begin(); i != m_exclusive.end(); ++i)
				valueSet.set(*i, asFloat(false));
		}
	}
}

void IfInclusiveExclusive::serialize(ISerializer& s)
{
	const MemberEnum< Priority >::Key c_Priority_Keys[] =
	{
		{ L"PrInclusive", PrInclusive },
		{ L"PrExclusive", PrExclusive },
		0
	};
	
	s >> MemberEnum< Priority >(L"priority", m_priority, c_Priority_Keys);
	s >> MemberStlList< std::wstring >(L"inclusive", m_inclusive);
	s >> MemberStlList< std::wstring >(L"exclusive", m_exclusive);
}

	}
}
