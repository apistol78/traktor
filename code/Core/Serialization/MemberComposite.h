#pragma once

#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

/*! \brief Composite member.
 * \ingroup Core
 */
template < typename Class, bool Compound = true >
class MemberComposite : public MemberComplex
{
public:
	typedef Class value_type;

	MemberComposite(const wchar_t* const name, value_type& ref)
	:	MemberComplex(name, Compound)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		m_ref.serialize(s);
	}

private:
	value_type& m_ref;
};

}

